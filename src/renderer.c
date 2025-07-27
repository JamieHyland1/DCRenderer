#include <stdio.h>
#include "../include/mesh.h"
#include "../include/vector.h"
#include "../include/display.h"
#include "../include/mesh.h"
#include "../include/triangle.h"
#include "../include/camera.h"
#include "../include/array.h"
#include "../include/matrix.h"
#include "../include/light.h"
#include "../include/clipping.h"
#include <malloc.h>
#include <dc/perf_monitor.h>
#include <kos.h>
#include <dc/perfctr.h>
#include <dc/video.h>

#define MAX_TRIANGLES_PER_MESH 10000

int frame_count = 0;
int previous_frame_time = 0;
int num_triangles_to_render = 0;
float delta_time;
float global_y_rotation = 0.0f; // Global rotation angle
bool isRunning = false;
mat4_t world_matrix;
mat4_t projection_matrix;
mat4_t view_matrix;
triangle_t triangles_to_render[MAX_TRIANGLES_PER_MESH];
uint64_t cycles = 0;

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



bool setup(void){
    vid_set_mode(DM_640x480 | DM_MULTIBUFFER, PM_RGB565);
    vid_mode->fb_count = 4;
    printf("Framebuffer count after vid_set_mode: %d\n", vid_mode->fb_count);
    printf("Vid get start width: %ld\n", vid_get_start(-1));

    //vid_set_dithering(true);
    render_mode = RENDER_TEXTURED;
    cull_mode = CULL_BACKFACE;
    // Initialize Light Direction
    init_light(vec3_new(0, 0, 1));

    // Initialize projection matrix
    float aspect_x = (float)get_window_width() / (float)get_window_height();
    float aspect_y = (float)get_window_height() / (float)get_window_width();

    float fov_y = M_PI / 3.0;
    float fov_x = atan(tan(fov_y / 2) * aspect_x) * 2;

    float znear = 0.1;
    float zfar = 100.0;
    projection_matrix = mat4_make_perspective(fov_y, aspect_y, znear, zfar);

    init_frustum_planes(fov_x, fov_y, znear, zfar);
    load_mesh("rd/cube.obj", "rd/cube.png", vec3_new(1, 1, 1), vec3_new(-3, 0, 18), vec3_new(0, 0, 0));
    load_mesh("rd/skull.obj", "rd/SamTexture.png", vec3_new(8,8,8), vec3_new(3,0,18), vec3_new(0,0,0));

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
void process_graphics_pipeline(mesh_t *mesh){
    vec3f_t target = get_camera_lookat_target();

    view_matrix = mat4_look_at(get_camera_pos(), target, get_camera_up());

    mat4_t scale_matrix = mat4_make_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
    mat4_t translation_matrix = mat4_make_translation(mesh->translation.x, mesh->translation.y, mesh->translation.z);

    mat4_t rotation_matrix_x = mat4_rotate_x(mesh->rotation.x);
    mat4_t rotation_matrix_y = mat4_rotate_y(mesh->rotation.y += 0.02);
    mat4_t rotation_matrix_z = mat4_rotate_z(mesh->rotation.z);

    world_matrix = mat4_identity();
            
    // Multiply Scale -> Rotation -> Translation ORDER MATTERS >:(
    world_matrix = matrix_mult_mat4(scale_matrix, world_matrix);
    world_matrix = matrix_mult_mat4(rotation_matrix_z, world_matrix);
    world_matrix = matrix_mult_mat4(rotation_matrix_y, world_matrix);
    world_matrix = matrix_mult_mat4(rotation_matrix_x, world_matrix);
    world_matrix = matrix_mult_mat4(translation_matrix, world_matrix);
   
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

            // Create a world matrix
            current_vertex = matrix_mult_vec4(world_matrix, current_vertex);
            current_vertex = matrix_mult_vec4(view_matrix, current_vertex);
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

        // // printf("number of vertices after clipping: %d\n", polygon.num_vertices);

        // Break clipped polygon into triangles to be rendered
        triangle_t tris_after_clipping[MAX_NUM_POLY_TRIS];
        int num_triangles_after_clipping = 0;

        triangles_from_polygon(&polygon, tris_after_clipping, &num_triangles_after_clipping);

        // Loop through triangles after clipping

        for (int t = 0; t < num_triangles_after_clipping; t++)
        {

            triangle_t triangle_after_clipping = tris_after_clipping[t];

            vector_t projected_points[3];

            for (int j = 0; j < 3; j++)
            {
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
                .texture = mesh->img
            };
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
    num_triangles_to_render = 0;
   // global_y_rotation += 0.01f; // Increment rotation angle each frame
    for (int mesh_index = 0; mesh_index < get_num_meshes(); mesh_index++){
        mesh_t* mesh = get_mesh(mesh_index);
        #ifdef DEBUG_CYCLES 
            perf_cntr_clear(PRFC0);
            perf_cntr_start(PRFC0, PMCR_INSTRUCTION_CACHE_MISS_MODE, PMCR_COUNT_CPU_CYCLES);
        #endif
        process_graphics_pipeline(mesh);
        #ifdef DEBUG_CYCLES 
            perf_cntr_stop(PRFC0);
            cycles = perf_cntr_count(PRFC0);
            printf("Process Graphics Pipeline: %llu cycles\n", cycles);
    #endif
      
    }
}

void process_input(void)
{
    #ifdef DEBUG_CYCLES 
        perf_cntr_clear(PRFC0);
        perf_cntr_start(PRFC0, PMCR_INSTRUCTION_CACHE_MISS_MODE, PMCR_COUNT_CPU_CYCLES);
    #endif

    maple_device_t *cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
    if (cont)
    {
        cont_state_t *state = (cont_state_t *)maple_dev_status(cont);
        if (!state) return;

        uint32_t buttons = state->buttons;

        #define PRESSED(btn) ((buttons & (btn)) && !(prev_buttons & (btn)))

        if (PRESSED(CONT_A)) {
            // TODO
        }
        if (PRESSED(CONT_B)) {
            render_mode++;
        }
        if (PRESSED(CONT_X)) {
            render_mode--;
        }
        if (PRESSED(CONT_Y)) {
            isRunning = false;
        }
        if (state->buttons & CONT_DPAD_UP)
        {
            set_camera_vel(vec3_mult(get_camera_dir(),0.2f));
            set_camera_pos(vec3_add(get_camera_pos(),get_camera_vel()));
        }
        if (state->buttons & CONT_DPAD_DOWN)
        {
            set_camera_vel(vec3_mult(get_camera_dir(),0.2f));
            set_camera_pos(vec3_sub(get_camera_pos(),get_camera_vel()));
        }
        if (state->buttons & CONT_DPAD_LEFT)
        {
            vec3f_t vel = (vec3_cross(get_camera_dir(), get_camera_up()));
            vel = vec_normalize(vel);
            vel = vec3_mult(vel,0.2f);
            set_camera_vel(vel);
            set_camera_pos(vec3_add(get_camera_pos(),get_camera_vel()));
        }
        if (state->buttons & CONT_DPAD_RIGHT)
        {
            vec3f_t vel = (vec3_cross(get_camera_dir(), get_camera_up()));
            vel = vec_normalize(vel);
            vel = vec3_mult(vel,0.2f);
            set_camera_vel(vel);
            set_camera_pos(vec3_sub(get_camera_pos(),get_camera_vel()));
        }

        // Trigger edge detection
        if (state->rtrig > 200 && prev_rtrig <= 200) {
            render_mode++;
        }
        if (state->ltrig > 200 && prev_ltrig <= 200) {
            render_mode--;
        }

        if (render_mode < 0) {
            render_mode = 5;
        } else if (render_mode > 5) {
            render_mode = 0;
        }

        prev_buttons = buttons;
        prev_ltrig = state->ltrig;
        prev_rtrig = state->rtrig;
    }

    #ifdef DEBUG_CYCLES 
        perf_cntr_stop(PRFC0);
        cycles = perf_cntr_count(PRFC0);
        printf("Input handling: %llu cycles\n", cycles);
    #endif
}


void render(void)
{  
     #ifdef DEBUG_TIME
        perf_monitor();
    #endif

    vid_clear(25, 25, 25);
    clear_z_buffer();
    
  

    for(int i = 0; i < num_triangles_to_render; i ++){
        triangle_t tri = triangles_to_render[i];
        switch(render_mode)
        {
            case RENDER_WIRE:
                #ifdef DEBUG_CYCLES 
                    perf_cntr_clear(PRFC0);
                    perf_cntr_start(PRFC0, PMCR_INSTRUCTION_CACHE_MISS_MODE, PMCR_COUNT_CPU_CYCLES);
                #endif
                draw_triangle(
                    tri.points[0].x, tri.points[0].y, // vertex A
                    tri.points[1].x, tri.points[1].y, // vertex B
                    tri.points[2].x, tri.points[2].y, // vertex C
                    0xFFFF // color (16-bit white)
                );
                #ifdef DEBUG_CYCLES 
                    perf_cntr_stop(PRFC0);
                    cycles = perf_cntr_count(PRFC0);
                    printf("Draw Wire Triangle: %llu cycles\n", cycles);
                #endif
                break;
            case RENDER_FILL_TRIANGLE:
                #ifdef DEBUG_CYCLES 
                    perf_cntr_clear(PRFC0);
                    perf_cntr_start(PRFC0, PMCR_INSTRUCTION_CACHE_MISS_MODE, PMCR_COUNT_CPU_CYCLES);
                #endif
                draw_filled_triangle(
                    &(vec2i_t){(int)tri.points[0].x, (int)tri.points[0].y},
                    &(vec2i_t){(int)tri.points[1].x, (int)tri.points[1].y},
                    &(vec2i_t){(int)tri.points[2].x, (int)tri.points[2].y},
                    0xF800 // Red color
                );
                #ifdef DEBUG_CYCLES 
                    perf_cntr_stop(PRFC0);
                    cycles = perf_cntr_count(PRFC0);
                    printf("Draw Fill Triangle: %llu cycles\n", cycles);
                #endif
                break;
            case RENDER_FILL_TRIANGLE_WIRE:
                #ifdef DEBUG_CYCLES 
                    perf_cntr_clear(PRFC0);
                    perf_cntr_start(PRFC0, PMCR_INSTRUCTION_CACHE_MISS_MODE, PMCR_COUNT_CPU_CYCLES);
                #endif
                draw_filled_triangle_wire(
                    &(vec2i_t){(int)tri.points[0].x, (int)tri.points[0].y},
                    &(vec2i_t){(int)tri.points[1].x, (int)tri.points[1].y},
                    &(vec2i_t){(int)tri.points[2].x, (int)tri.points[2].y},
                    0xF800 // Red color
                );
                #ifdef DEBUG_CYCLES 
                    perf_cntr_stop(PRFC0);
                    cycles = perf_cntr_count(PRFC0);
                    printf("Draw filled Tirangle Wire: %llu cycles\n", cycles);
                #endif
                break;
            case RENDER_TEXTURED:
                #ifdef DEBUG_CYCLES 
                    perf_cntr_clear(PRFC0);
                    perf_cntr_clear(PRFC1);
                    perf_cntr_start(PRFC0, PMCR_ELAPSED_TIME_MODE, PMCR_COUNT_CPU_CYCLES);
                    perf_cntr_start(PRFC1, PMCR_OPERAND_CACHE_READ_MISS_MODE, PMCR_COUNT_CPU_CYCLES);
                #endif
                draw_textured_triangle(
                    &(vec2i_t){(int)tri.points[0].x, (int)tri.points[0].y},
                    &(vec2i_t){(int)tri.points[1].x, (int)tri.points[1].y},
                    &(vec2i_t){(int)tri.points[2].x, (int)tri.points[2].y},
                    &(vec2_t){   tri.texcoords[0].u,   tri.texcoords[0].v},
                    &(vec2_t){   tri.texcoords[1].u,   tri.texcoords[1].v},
                    &(vec2_t){   tri.texcoords[2].u,   tri.texcoords[2].v},
                    tri.texture
                );
                #ifdef DEBUG_CYCLES 
                    perf_cntr_stop(PRFC0);
                    perf_cntr_stop(PRFC1);
                    uint64_t cycles = perf_cntr_count(PRFC0);
                    uint64_t read_misses = perf_cntr_count(PRFC1);
                    printf("Draw Textured Triangle: %llu cycles, %llu read misses\n", cycles, read_misses);
                #endif
                break;
            case RENDER_TEXTURED_WIRE:
                #ifdef DEBUG_CYCLES 
                    perf_cntr_clear(PRFC0);
                    perf_cntr_start(PRFC0, PMCR_INSTRUCTION_CACHE_MISS_MODE, PMCR_COUNT_CPU_CYCLES);
                #endif
                // TODO draw_textured_triangle_wire
                #ifdef DEBUG_CYCLES 
                    perf_cntr_stop(PRFC0);
                    cycles = perf_cntr_count(PRFC0);
                    printf("Draw Textured Triangle Wire: %llu cycles\n", cycles);
                #endif
                break;
        }

        draw_info(render_mode, num_triangles_to_render);

    }
 
    #ifdef DEBUG_TIME
        perf_monitor_print(stdout);
        perf_monitor_exit();
        if(frame_count == 1)isRunning = false; // Stop after 2 frames for testing
    #endif

    #ifdef DEBUG_CYCLES
        if(frame_count == 1)isRunning = false; // Stop after 2 frames for testing
    #endif
}

int main(int argc, char *args[])
{
    isRunning = initialize_window();

    setup();
   
    while (isRunning)
    {
        vid_waitvbl();
        process_input();
        update();
        render();
        vid_flip(vid_mode->fb_count);
        frame_count++;
       
    }

    
    free_meshes();
    destroy_window();

    return 0;
}
