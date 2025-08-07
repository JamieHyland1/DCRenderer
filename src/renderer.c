#include <stdio.h>
#include "../include/renderer.h"

#define MAX_TRIANGLES_PER_MESH 10000
#define F_PI 3.1415926f
int frame_count = 0;
int previous_frame_time = 0;
int num_triangles_to_render = 0;
float delta_time;
float global_y_rotation = 0.0f; // Global rotation angle
bool isRunning = false;
mat4_t world_matrix;
mat4_t projection_matrix;
mat4_t view_matrix;

matrix_t v_mat;
matrix_t w_mat;
matrix_t p_mat;
triangle_t triangles_to_render[MAX_TRIANGLES_PER_MESH];


static uint32_t prev_buttons = 0;
static uint8_t prev_ltrig = 0;
static uint8_t prev_rtrig = 0;

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
    RENDER_TEXTURED_WIRE
} render_method;

enum render_method render_mode;
enum cull_method cull_mode;

static inline vector_t vec4_from_vec3f(vec3f_t v)
{
    return (vector_t){v.x, v.y, v.z, 1.0f};
}

bool setup(void)
{
    vid_set_mode(DM_640x480 | DM_MULTIBUFFER, PM_RGB565);
    vid_mode->fb_count = 2;
    printf("Framebuffer count after vid_set_mode: %d\n", vid_mode->fb_count);
    buffer_size = 640 * 480 * sizeof(uint16_t);
    buffer = (uint16_t *)aligned_alloc(32, buffer_size);
    z_buffer = (float *)malloc(sizeof(float) * 640 * 480);

    // check_mesh_size();
    render_mode = RENDER_TEXTURED;
    cull_mode = CULL_BACKFACE;

    // Initialize projection matrix
    float aspect_x = (float)get_window_width() / (float)get_window_height();
    float aspect_y = (float)get_window_height() / (float)get_window_width();

    float aspect_ratio = (float)get_window_width() / (float)get_window_height();
    float fov_y = F_PI / 3.0f;
    float fov_x = atanf(tanf(fov_y / 2) * aspect_x) * 2;
    float cot_fovy_2 = 1.0f / tanf(fov_y / 2.0f);
    float znear = 0.1f;
    float zfar = 1000.0f;
    mat_identity();
    mat_perspective(aspect_ratio, 1.0f, cot_fovy_2, znear, zfar);
    mat_store(&p_mat);

    projection_matrix = mat4_make_perspective(fov_y, aspect_y, znear, zfar);
    init_frustum_planes(fov_x, fov_y, znear, zfar);

    // too slow right now for skybox
    //  load_mesh("rd/skybox.obj", "rd/skybox_yokohama.png", vec3_new(1, 1, 1), vec3_new(0, 0, 0), vec3_new(0, 0, 0));
    load_mesh("rd/cube.obj", "rd/cube.png", vec3_new(1, 1, 1), vec3_new(0, 0, -14), vec3_new(0, 0, 0));
    set_camera_pos((vec3f_t){0, 0, 0});
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
    mat_identity();
    mat_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
    mat_translate(mesh->translation.x, mesh->translation.y, mesh->translation.z);
    mat_rotate_z(mesh->rotation.z -= 0.01);
    mat_rotate_y(mesh->rotation.y += 0.2f);
    mat_rotate_x(mesh->rotation.x += 0.02f);
    mat_store(&w_mat);

    mat_load(&v_mat);
    mat_apply(&w_mat);

    int num_faces = array_length(mesh->faces);
    for (int i = 0; i < num_faces; i++)
    {

        face_t current_face = mesh->faces[i];

        vec3f_t face_vertices[3];

        face_vertices[0] = mesh->vertices[current_face.a];
        face_vertices[1] = mesh->vertices[current_face.b];
        face_vertices[2] = mesh->vertices[current_face.c];

        vector_t transformed_vertices[3];

        for (int j = 0; j < 3; j++)
        {
            vector_t current_vertex = vec4_from_vec3(face_vertices[j]);

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
        vec3f_t origin = {0, 0, 0};
        /////////////////
        // Cull back faces
        /////////////////
        vec3f_t cameraRay = vec3_sub(origin, vec3_from_vec4(transformed_vertices[0]));
        vec3f_t tri_normal = get_triangle_face_normal(transformed_vertices);
        tri_normal = vec_normalize(tri_normal);

        float orientation_from_camera = vec_dot(tri_normal, cameraRay);
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

        // Clip polygon with the view frustum
        clip_polygon(&polygon);

        // Break clipped polygon into triangles to be rendered
        triangle_t tris_after_clipping[MAX_NUM_POLY_TRIS];
        int num_triangles_after_clipping = 0;

        triangles_from_polygon(&polygon, tris_after_clipping, &num_triangles_after_clipping);

        for (int t = 0; t < num_triangles_after_clipping; t++)
        {

            triangle_t triangle_after_clipping = tris_after_clipping[t];

            vector_t projected_points[3];
            for (int j = 0; j < 3; j++)
            {
                // TODO figure out how to use KOS perspective matrix
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
                projected_points[j].x *= get_window_width() * 0.5f;
                projected_points[j].y *= get_window_height() * 0.5f;

                // Invert Y values to account for flipped y screen coordinate
                projected_points[j].y *= -1;

                // Translate to middle of screen
                projected_points[j].x += get_window_width() * 0.5f;
                projected_points[j].y += get_window_height() * 0.5f;
            }
            triangle_t triangle_to_render = {
                .points = {
                    {projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w},
                    {projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w},
                    {projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w},
                },
                .texcoords = {{triangle_after_clipping.texcoords[0].u, triangle_after_clipping.texcoords[0].v}, {triangle_after_clipping.texcoords[1].u, triangle_after_clipping.texcoords[1].v}, {triangle_after_clipping.texcoords[2].u, triangle_after_clipping.texcoords[2].v}},
                .texture = mesh->img};
            if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH)
            {
                triangles_to_render[num_triangles_to_render] = triangle_to_render;
                num_triangles_to_render++;
            }
        }
    }
}

void process_skybox(mesh_t *mesh)
{
    mat_identity();

    int num_faces = array_length(mesh->faces);
    for (int i = 0; i < num_faces; i++)
    {
        face_t current_face = mesh->faces[i];

        vec3f_t face_vertices[3] = {
            mesh->vertices[current_face.a],
            mesh->vertices[current_face.b],
            mesh->vertices[current_face.c]};

        vector_t transformed_vertices[3];

        vec3f_t origin = {0, 0, 0};
        /////////////////
        // Cull back faces
        /////////////////
        vec3f_t cameraRay = vec3_sub(origin, vec3_from_vec4(transformed_vertices[0]));
        vec3f_t tri_normal = get_triangle_face_normal(transformed_vertices);
        tri_normal = vec_normalize(tri_normal);

        float orientation_from_camera = vec_dot(tri_normal, cameraRay);
        if (cull_mode == CULL_BACKFACE)
        {
            if (orientation_from_camera < 0)
            {
                continue;
            }
        }

        for (int j = 0; j < 3; j++)
        {
            vector_t current_vertex = vec4_from_vec3(face_vertices[j]);
            float x = current_vertex.x;
            float y = current_vertex.y;
            float z = current_vertex.z;

            mat_trans_single3_nodiv(x, y, z);

            current_vertex.x = x;
            current_vertex.y = y;
            current_vertex.z = z;
            transformed_vertices[j] = current_vertex;
        }

        vector_t projected_points[3];
        for (int j = 0; j < 3; j++)
        {
            projected_points[j] = mat4_mul_vec4_project(projection_matrix, transformed_vertices[j]);

            projected_points[j].x *= get_window_width() * 0.5f;
            projected_points[j].y *= get_window_height() * 0.5f;

            projected_points[j].y *= -1;

            projected_points[j].x += get_window_width() * 0.5f;
            projected_points[j].y += get_window_height() * 0.5f;
        }

        // Create triangle
        triangle_t triangle_to_render = {
            .points = {
                projected_points[0],
                projected_points[1],
                projected_points[2]},
            .texcoords = {current_face.a_uv, current_face.b_uv, current_face.c_uv},
            .texture = mesh->img};

        if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH)
        {
            triangles_to_render[num_triangles_to_render++] = triangle_to_render;
        }
    }
}

void update(void)
{
    num_triangles_to_render = 0;
    mat_identity();

    vector_t cam_pos = vec4_from_vec3f(get_camera_pos());
    vector_t cam_target = vec4_from_vec3f(get_camera_lookat_target());
    vector_t cam_up = vec4_from_vec3f(get_camera_up());

    // View matrix
    mat_lookat(&cam_pos, &cam_target, &cam_up);
    mat_store(&v_mat);

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
            // TODO
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
        }
        if (state->buttons & CONT_DPAD_UP)
        {
            set_camera_vel(vec3_mult(get_camera_dir(), -0.2f));
            set_camera_pos(vec3_add(get_camera_pos(), get_camera_vel()));
        }
        if (state->buttons & CONT_DPAD_DOWN)
        {
            set_camera_vel(vec3_mult(get_camera_dir(), -0.2f));
            set_camera_pos(vec3_sub(get_camera_pos(), get_camera_vel()));
        }
        if (state->buttons & CONT_DPAD_LEFT)
        {
            vec3f_t vel = (vec3_cross(get_camera_dir(), get_camera_up()));
            vel = vec_normalize(vel);
            vel = vec3_mult(vel, -0.2f);
            set_camera_vel(vel);
            set_camera_pos(vec3_add(get_camera_pos(), get_camera_vel()));
        }
        if (state->buttons & CONT_DPAD_RIGHT)
        {
            vec3f_t vel = (vec3_cross(get_camera_dir(), get_camera_up()));
            vel = vec_normalize(vel);
            vel = vec3_mult(vel, -0.2f);
            set_camera_vel(vel);
            set_camera_pos(vec3_sub(get_camera_pos(), get_camera_vel()));
        }

        // Trigger edge detection
        if (state->rtrig > 200 && prev_rtrig <= 200)
        {
            render_mode++;
        }
        if (state->ltrig > 200 && prev_ltrig <= 200)
        {
            render_mode--;
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
        int x1 = (int)tri.points[0].x;
        int y1 = (int)tri.points[0].y;
        int x2 = (int)tri.points[1].x;
        int y2 = (int)tri.points[1].y;
        int x3 = (int)tri.points[2].x;
        int y3 = (int)tri.points[2].y;

        if (x1 < minWindowX)
            minWindowX = x1;
        if (x1 > maxWindowX)
            maxWindowX = x1;
        if (y1 < minWindowY)
            minWindowY = y1;
        if (y1 > maxWindowY)
            maxWindowY = y1;

        if (x2 < minWindowX)
            minWindowX = x2;
        if (x2 > maxWindowX)
            maxWindowX = x2;
        if (y2 < minWindowY)
            minWindowY = y2;
        if (y2 > maxWindowY)
            maxWindowY = y2;

        if (x3 < minWindowX)
            minWindowX = x3;
        if (x3 > maxWindowX)
            maxWindowX = x3;
        if (y3 < minWindowY)
            minWindowY = y3;
        if (y3 > maxWindowY)
            maxWindowY = y3;

        switch (render_mode)
        {
        case RENDER_WIRE:
            draw_triangle(
                tri.points[0].x, tri.points[0].y, // vertex A
                tri.points[1].x, tri.points[1].y, // vertex B
                tri.points[2].x, tri.points[2].y, // vertex C
                0xFFFF                            // color (16-bit white)
            );
            break;
        case RENDER_FILL_TRIANGLE:
            draw_filled_triangle(
                &(vec2i_t){(int)tri.points[0].x, (int)tri.points[0].y},
                &(vec2i_t){(int)tri.points[1].x, (int)tri.points[1].y},
                &(vec2i_t){(int)tri.points[2].x, (int)tri.points[2].y},
                0xF800 // Red color
            );
            break;
        case RENDER_FILL_TRIANGLE_WIRE:
            draw_filled_triangle_wire(
                &(vec2i_t){(int)tri.points[0].x, (int)tri.points[0].y},
                &(vec2i_t){(int)tri.points[1].x, (int)tri.points[1].y},
                &(vec2i_t){(int)tri.points[2].x, (int)tri.points[2].y},
                0xF800 // Red color
            );

            break;
        case RENDER_TEXTURED:
            draw_textured_triangle(
                &(vec2i_t){(int)tri.points[0].x, (int)tri.points[0].y},
                &(vec2i_t){(int)tri.points[1].x, (int)tri.points[1].y},
                &(vec2i_t){(int)tri.points[2].x, (int)tri.points[2].y},
                &(vec2_t){tri.texcoords[0].u, tri.texcoords[0].v},
                &(vec2_t){tri.texcoords[1].u, tri.texcoords[1].v},
                &(vec2_t){tri.texcoords[2].u, tri.texcoords[2].v},
                tri.texture);

            break;
        }
    }

    //    // Top edge
    //     draw_line(minWindowX, minWindowY, maxWindowX, minWindowY, 0xFFFF);

    //     // Bottom edge
    //     draw_line(minWindowX, maxWindowY, maxWindowX, maxWindowY, 0xFFFF);

    //     // Left edge
    //     draw_line(minWindowX, minWindowY, minWindowX, maxWindowY, 0xFFFF);

    //     // Right edge
    //     draw_line(maxWindowX, minWindowY, maxWindowX, maxWindowY, 0xFFFF);
    draw_info(render_mode, num_triangles_to_render);
}

int main(int argc, char *args[])
{
    isRunning = initialize_window();

    setup();

    while (isRunning)
    {
        minWindowX = 640;
        minWindowY = 480;
        maxWindowX = 0;
        maxWindowY = 0;
        vid_waitvbl();
        process_input();
        update();
        render();
        sq_cpy((void *)((uint8_t *)vram_s), (const void *)((uint8_t *)buffer), buffer_size);
        vid_flip(vid_mode->fb_count);
        memset(buffer, get_background_color(), buffer_size);
        memset(z_buffer, 0, (640 * 480) * sizeof(float));
        frame_count++;
    }

    free_meshes();
    destroy_window();

    return 0;
}