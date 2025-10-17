#include <stdio.h>
#include "../include/renderer.h"
#include "shz_trig.h"
#include "shz_matrix.h"
#include "shz_xmtrx.h"
#include "shz_fpscr.h"
#define MAX_TRIANGLES_PER_MESH 10000

int frame_count = 0;
int previous_frame_time = 0;
int num_triangles_to_render = 0;

bool render_z_buffer = false;

mat4_t world_matrix;
mat4_t projection_matrix;
mat4_t view_matrix;

matrix_t v_mat;
matrix_t w_mat;
matrix_t p_mat;
matrix_t pv_mat;
triangle_t triangles_to_render[MAX_TRIANGLES_PER_MESH];
uint64_t triangle_time = 0;
uint64_t total_cycles = 0;

static uint32_t prev_buttons = 0;
static uint8_t prev_ltrig = 0;
static uint8_t prev_rtrig = 0;
uint64_t elapsed_time = 0;

// TODO pull these enums to a header file
enum cull_method
{
    CULL_NONE,
    CULL_BACKFACE

} cull_method;


enum render_method
{
    RENDER_WIRE,
    RENDER_FILL_TRIANGLE,
    RENDER_FILL_TRIANGLE_WIRE,
    RENDER_TEXTURED,
    RENDER_TEXTURED_SCANLINE
} render_method;

enum render_method render_mode;
enum cull_method cull_mode;

float znear = 0.1f;
static inline shz_vec4_t vec4_from_vec3f(shz_vec3_t v)
{
    return (shz_vec4_t){v.x, v.y, v.z, 1.0f};
}

bool setup(void)
{
    vid_set_mode(DM_640x480 | DM_MULTIBUFFER, PM_RGB565);
    vid_mode->fb_count = 2;

    printf("Framebuffer count after vid_set_mode: %d\n", vid_mode->fb_count);
    printf("Framebuffer size: %d bytes\n", vid_mode->fb_size);
    printf("Scanlines: %d pixels\n", vid_mode->scanlines);

    buffer_size = 640 * 480 * sizeof(uint16_t);
    buffer = (uint16_t *)aligned_alloc(32, buffer_size);
    background_texture = (uint16_t *)aligned_alloc(32, buffer_size);
    z_buffer = (float *)aligned_alloc(32, sizeof(float) * WINDOW_WIDTH * WINDOW_HEIGHT);

    load_background_image("rd/background2.png");

    init_light((shz_vec3_t){0.0f, 0.0f, -1.0f});

    render_mode = RENDER_TEXTURED;
    cull_mode = CULL_BACKFACE;

    // Initialize projection matrix
    float aspect_x = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    float aspect_y = (float)WINDOW_HEIGHT / (float)WINDOW_WIDTH;

    float aspect_ratio = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
    float fov_y = SHZ_F_PI / 3.0f;
    float fov_x = atanf(tanf(fov_y / 2) * aspect_x) * 2;
    float cot_fovy_2 = 1.0f / tanf(fov_y / 2.0f);
    float znear = 0.1f;
    float zfar = 1000.0f;

    mat_identity();
    mat_perspective(aspect_ratio, 1.0f, cot_fovy_2, znear, zfar);
    mat_store(&p_mat);

    projection_matrix = mat4_make_perspective(fov_y, aspect_y, znear, zfar);
    init_frustum_planes(fov_x, fov_y, znear, zfar);

                                                    // scale          position         rotation
    load_mesh("rd/cube.obj", "rd/cube.png", vec3_new(1, 1, 1), vec3_new( 0,  0,  -6), vec3_new(0, 0, 0));
    load_mesh("rd/cube.obj", "rd/cube.png", vec3_new(1, 1, 1), vec3_new( -2,  0,  -4), vec3_new(0, 0, 0));

    // set_camera_pos((shz_vec3_t){3, -2.6, 43.9f}); // TESTING POSITION
     set_camera_pos((shz_vec3_t){-1.6, 0.28, -1});
    return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Process the graphics pipeline in various stages
//
//  [ MODEL SPACE ] <- Original mesh vertices, all centered around the origin 0,0,0
//
//       [ WORLD SPACE ] <- Multiply vertices by world matrix, essentially "places" meshes in the correct spot in the scene
//
//           [ CAMERA SPACE ] <- Multiple vertices by the view matrix, essentially putting them in view relative to the camera
//
//               [ CLIPPING ] <- Clip any vertices in the meshes that are outside the bounds of the frustum plane
//
//                   [ PROJECTION ] <- Multiple by the projection matrix, flatten the 3D world into a 2D image
//
//                       [ IMAGE SPACE ] <- Apply perspective divide on the meshes adding depth to the scene
//
//                           [ SCREEN SPACE ] <- Final image is now ready to be rendered to the screen
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void process_graphics_pipeline(mesh_t *mesh)
{
    // World matrix
    shz_xmtrx_init_identity();
    shz_xmtrx_apply_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
    shz_xmtrx_apply_translation(mesh->translation.x, mesh->translation.y, mesh->translation.z);
    shz_xmtrx_apply_rotation_z(mesh->rotation.z);
    shz_xmtrx_apply_rotation_y(mesh->rotation.y += 0.01f);
    shz_xmtrx_apply_rotation_x(mesh->rotation.x );
    // its trying to replace these
    shz_xmtrx_store_4x4(&w_mat);
    shz_xmtrx_load_4x4(&v_mat);
    shz_xmtrx_apply_4x4(&w_mat);
    //
    // shz_xmtrx_load_4x4_apply_store(&pv_mat, &v_mat, &w_mat);
    // shz_xmtrx_load_4x4(&w_mat);
    // shz_xmtrx_invert();
    // shz_xmtrx_transpose();


    int num_faces = array_length(mesh->faces);
    for (int i = 0; i < num_faces; i++)
    {

        face_t current_face = mesh->faces[i];

        shz_vec3_t face_vertices[3];

        face_vertices[0] = mesh->vertices[current_face.a];
        face_vertices[1] = mesh->vertices[current_face.b];
        face_vertices[2] = mesh->vertices[current_face.c];

        shz_vec4_t transformed_vertices[3];

        for (int j = 0; j < 3; j++)
        {
            shz_vec4_t current_vertex = vec4_from_vec3(face_vertices[j]);

            float x = current_vertex.x;
            float y = current_vertex.y;
            float z = current_vertex.z;
            float w = current_vertex.w;

            mat_trans_single3_nodiv(x, y, z);

            current_vertex.x = x;
            current_vertex.y = y;
            current_vertex.z = z;
            current_vertex.w = w;
            transformed_vertices[j] = current_vertex;
        }
        shz_vec3_t origin = shz_vec3_init(0, 0, 0);
        /////////////////
        // Cull back faces
        /////////////////
        shz_vec3_t cameraRay = shz_vec_sub(origin, vec3_from_vec4(transformed_vertices[0]));
        shz_vec3_t tri_normal = get_triangle_face_normal(transformed_vertices);
        tri_normal = shz_vec_normalize(tri_normal);

        float orientation_from_camera = shz_vec_dot(tri_normal, cameraRay);
        if (cull_mode == CULL_BACKFACE)
        {
            if (orientation_from_camera < 0)
            {
                continue;
            }
        }

        ////////////////
        // Clipping stage
        ////////////////

        // Create a polygon from the transformed triangle
        polygon_t polygon = create_polygon_from_triangle(
            vec3_from_vec4(transformed_vertices[0]),
            vec3_from_vec4(transformed_vertices[1]),
            vec3_from_vec4(transformed_vertices[2]),
            current_face.a_uv,
            current_face.b_uv,
            current_face.c_uv);


        // TODO refactor this to use FIPVR 
        clip_polygon(&polygon);


        // Break clipped polygon into triangles to be rendered
        triangle_t tris_after_clipping[MAX_NUM_POLY_TRIS];
        int num_triangles_after_clipping = 0;

        triangles_from_polygon(&polygon, tris_after_clipping, &num_triangles_after_clipping);

        for (int t = 0; t < num_triangles_after_clipping; t++)
        {

            triangle_t triangle_after_clipping = tris_after_clipping[t];

            shz_vec4_t projected_points[3];
            for (int j = 0; j < 3; j++)
            {
                // TODO switch to sh4zam mtrx code
                //  float x = triangle_after_clipping.points[j].x;
                //  float y = triangle_after_clipping.points[j].y;
                //  float z = triangle_after_clipping.points[j].z;
                //  float w = triangle_after_clipping.points[j].w;

                // mat_trans_single4(x, y, z, w);
                // //mat_trans_single3_nomod(x, y, z, projected_points[j].x, projected_points[j].y, projected_points[j].z);

                // projected_points[j].x = x / w;
                // projected_points[j].y = y / w;
                // projected_points[j].z = z / w;
                // projected_points[j].w = w / w;

                projected_points[j] = mat4_mul_vec4_project(projection_matrix, triangle_after_clipping.points[j]);

                // Scale to middle of screen
                projected_points[j].x *= WINDOW_WIDTH * 0.5f;
                projected_points[j].y *= WINDOW_HEIGHT * 0.5f;

                // Invert Y values to account for flipped y screen coordinate
                projected_points[j].y *= -1;

                // Translate to middle of screen
                projected_points[j].x += WINDOW_WIDTH * 0.5f;
                projected_points[j].y += WINDOW_HEIGHT * 0.5f;
            }

            ////////////////
            //Lighting stage  TODO; add proper lighiting when sh4zam gets inv_mtrx_transpose
            ////////////////

            //  shz_vec3_t tri_center = {
            //     (triangle_after_clipping.points[0].x +
            //     triangle_after_clipping.points[1].x +
            //     triangle_after_clipping.points[2].x) / 3.0f,
            //     (triangle_after_clipping.points[0].y +
            //     triangle_after_clipping.points[1].y +
            //     triangle_after_clipping.points[2].y) / 3.0f,
            //     (triangle_after_clipping.points[0].z +
            //     triangle_after_clipping.points[1].z +
            //     triangle_after_clipping.points[2].z) / 3.0f,
            // };

            // shz_vec3_t L0 = shz_vec_normalize(shz_vec_sub(point_lights[0].position, tri_center));
            // shz_vec3_t L1 = shz_vec_normalize(shz_vec_sub(point_lights[1].position, tri_center));
            // shz_vec3_t L2 = shz_vec_normalize(shz_vec_sub(point_lights[2].position, tri_center));
            // shz_vec3_t L3 = shz_vec_normalize(shz_vec_sub(point_lights[3].position, tri_center));
            

            // shz_matrix_4x4_t light_matrix = {
            //     .elem = {
            //         L0.x, L0.y, L0.z, 0.0f,
            //         L1.x, L1.y, L1.z, 0.0f,
            //         L2.x, L2.y, L2.z, 0.0f,
            //         L3.x, L3.y, L3.z, 0.0f
            //     }
            // };

            // shz_xmtrx_init_identity();
            // shz_xmtrx_load_4x4(&light_matrix);
           


            float orientation_from_light = shz_vec_dot(tri_normal, get_light_direction());
            orientation_from_light = orientation_from_light < 0.15 ? 0.15 : orientation_from_light;
            orientation_from_light = orientation_from_light > 1.00 ? 1.00 : orientation_from_light;
            

            triangle_t triangle_to_render = {
                .points = {
                    {projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w},
                    {projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w},
                    {projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w},
                },
                .texcoords = {{triangle_after_clipping.texcoords[0].u, triangle_after_clipping.texcoords[0].v}, {triangle_after_clipping.texcoords[1].u, triangle_after_clipping.texcoords[1].v}, {triangle_after_clipping.texcoords[2].u, triangle_after_clipping.texcoords[2].v}},
                .texture = mesh->img,
                .orientation_from_light = 0xFFFF};
            if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH)
            {
                triangles_to_render[num_triangles_to_render] = triangle_to_render;
                num_triangles_to_render++;
            }
        }
    }
}

void update(void)
{
    init_light((shz_vec3_t){0.0f, 0.0f, -1.0f});
    shz_xmtrx_init_identity();

    shz_vec4_t cam_pos = vec4_from_vec3f(get_camera_pos());
    shz_vec4_t cam_target = vec4_from_vec3f(get_camera_lookat_target());
    shz_vec4_t cam_up = vec4_from_vec3f(get_camera_up());

    // View matrix
    mat_lookat((vector_t*)&cam_pos,(vector_t*) &cam_target, (vector_t*)&cam_up);
    shz_xmtrx_store_4x4(&v_mat);

    for (int mesh_index = 0; mesh_index < get_num_meshes(); mesh_index++)
    {
        mesh_t *mesh = get_mesh(mesh_index);
        process_graphics_pipeline(mesh);

    }
}

void process_input(void)
{

    maple_device_t *cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
    if (cont)
    {
        cont_state_t *state = (cont_state_t *)maple_dev_status(cont);
        if (!state)
            return;

        uint32_t buttons = state->buttons;

        #define PRESSED(btn) ((buttons & (btn)) && !(prev_buttons & (btn)))

        if (PRESSED(CONT_A))
        {
            render_z_buffer = !render_z_buffer;
        }
        if (PRESSED(CONT_B))
        {
            render_mode++;
        }
        if (PRESSED(CONT_X))
        {
            render_mode--;
        }
        if (PRESSED(CONT_Y))
        {
            isRunning = false;
            printf("Y button pressed, setting isRunning to false at frame %d\n", frame_count);
        }
        if (state->buttons & CONT_DPAD_UP)
        {
            set_camera_vel(shz_vec3_scale(get_camera_dir(), -0.2f));
            set_camera_pos(shz_vec_add(get_camera_pos(), get_camera_vel()));
        }
        if (state->buttons & CONT_DPAD_DOWN)
        {
            set_camera_vel(shz_vec3_scale(get_camera_dir(), -0.2f));
            set_camera_pos(shz_vec_sub(get_camera_pos(), get_camera_vel()));
        }
        if (state->buttons & CONT_DPAD_LEFT)
        {
            shz_vec3_t vel = (shz_vec_cross(get_camera_dir(), get_camera_up()));
            vel = shz_vec_normalize(vel);
            vel = shz_vec3_scale (vel, -0.2f);
            set_camera_vel(vel);
            set_camera_pos(shz_vec_add(get_camera_pos(), get_camera_vel()));
        }
        if (state->buttons & CONT_DPAD_RIGHT)
        {
            shz_vec3_t vel = (shz_vec_cross(get_camera_dir(), get_camera_up()));
            vel = shz_vec_normalize(vel);
            vel = shz_vec3_scale(vel, -0.2f);
            set_camera_vel(vel);
            set_camera_pos(shz_vec_sub(get_camera_pos(), get_camera_vel()));
        }

        // Trigger edge detection
        if (state->rtrig > 0)
        {
           shz_vec3_t pos = get_camera_pos();
           pos.y += state->rtrig * 0.001f;
           set_camera_pos(pos);

        }
        if (state->ltrig > 200 )
        {
            shz_vec3_t pos = get_camera_pos();
            pos.y -= state->ltrig * 0.001f;
            set_camera_pos(pos);
        }

        if (render_mode < 0)
        {
            render_mode = 5;
        }
        else if (render_mode > 5)
        {
            render_mode = 0;
        }

        prev_buttons = buttons;
        prev_ltrig = state->ltrig;
        prev_rtrig = state->rtrig;
    }
}

void render(void)
{
   
    clear_z_buffer();
    for (int i = 0; i < num_triangles_to_render; i++)
    {
        triangle_t tri = triangles_to_render[i];
        switch (render_mode){
            case RENDER_WIRE:
                draw_triangle(&tri, 0xFFFF);
                break;
            case RENDER_FILL_TRIANGLE:
                draw_filled_triangle(&tri, 0xF800);
                break;
            case RENDER_FILL_TRIANGLE_WIRE:
                draw_filled_triangle_wire(&tri, 0xF800);
                break;
            case RENDER_TEXTURED:
                start_time = perf_cntr_timer_ns();
                draw_textured_triangle(&tri);
                end_time = perf_cntr_timer_ns();
                avg += end_time - start_time;
                break;
            case RENDER_TEXTURED_SCANLINE:
                start_time = perf_cntr_timer_ns();
                draw_textured_triangle_scanline(&tri);
                end_time = perf_cntr_timer_ns();
                avg += end_time - start_time;

                break;
        }
    }
    if(render_z_buffer == true){
        draw_z_buffer_to_screen();
    }
    draw_info(render_mode, num_triangles_to_render, frame_count);
    num_triangles_to_render = 0; // Reset for next frame

}

int main(int argc, char *args[])
{
    isRunning = initialize_window();

    setup();
    while (isRunning)
    {
        vid_flip(vid_mode->fb_count);
        draw_background_image();
        process_input();
        update();
        render();
        
        // Need to actually test if this is faster than kos sq_cpy
        sq_lock((void *)((uint8_t *)vram_s));
        shz_sq_memcpy32((void *)((uint8_t *)vram_s), (const void *)((uint8_t *)buffer), buffer_size);
        sq_unlock();

        // will keep this here for now until shz_sq_memcpy32 is fully tested
        // sq_cpy((void *)((uint8_t *)vram_s), (const void *)((uint8_t *)buffer), buffer_size);
       
        memset(z_buffer, 0, (WINDOW_WIDTH * WINDOW_HEIGHT) * sizeof(float));
        frame_count++;
        if(frame_count == 2000) isRunning = false;
    }
    
    // double average_ns = (double)avg / frame_count;  // Cast to double for floating-point division
    // printf("Average frame time: %.2f ns (%.2f ms)\n", average_ns, average_ns / 1e6);


    free_meshes();
    destroy_window();

    return 0;
}
