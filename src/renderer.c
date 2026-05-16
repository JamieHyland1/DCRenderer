#include <stdio.h>
#include "../include/renderer.h"
#include "dc/syscalls.h"
#include <stdlib.h>
#include "shz_trig.h"
#include "shz_matrix.h"
#include <dc/sq.h>
#include "shz_xmtrx.h"
#include "shz_fpscr.h"
#include "../include/core.h"
#include <dc/perfctr.h>
#include <dc/video.h>

bool isRunning = false;

int frame_count = 0;
int previous_frame_time = 0;
bool draw_z_buffer = false;
int skybox_tris_rendererd = 0;

uint64_t triangle_time = 0;
uint64_t total_cycles = 0;
uint64_t elapsed_time = 0;
int64_t dcache_freeze = 0;

enum draw_method draw_mode;

void load_background_image(const char *path);
int get_num_objects(void);

static uint64_t pipeline_total_ns = 0;
static uint64_t render_total_ns   = 0;
static uint64_t copy_total_ns     = 0;

bool setup(void)
{
    vid_set_mode(DM_640x480 | DM_MULTIBUFFER, PM_RGB565);
    vid_mode->fb_count = 2;

    printf("Framebuffer count after vid_set_mode: %d\n", vid_mode->fb_count);
    printf("Framebuffer size: %d bytes\n", vid_mode->fb_size);
    printf("Scanlines: %d pixels\n", vid_mode->scanlines);
    printf("hello from jamies renderer!\n");

    buffer_size = 640 * 480 * sizeof(uint16_t);
    buffer = (uint16_t *)aligned_alloc(32, buffer_size);
    background_texture = (uint16_t *)aligned_alloc(32, buffer_size);
    z_buffer = (float *)aligned_alloc(32, sizeof(float) * WINDOW_WIDTH * WINDOW_HEIGHT);

    load_background_image("rd/yokohama.png");

    init_light((shz_vec3_t){{{0.0f, 0.0f, -1.0f}}});

    draw_mode = DRAW_TEXTURED_SCANLINE;
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
    setup_plan_eq();

    // scale          position         rotation
    int skybox_id = load_assets("rd/Skybox.obj", "rd/SpeedHighway.png");
    (void)skybox_id;

    int cube_id = load_assets("rd/cube.obj", "rd/cube.png");

    for (int i = 0; i < 150; i++) {
        create_object(cube_id);
    }
    printf("size of object_t: %d bytes\n", sizeof(object_t));
    set_camera_pos((shz_vec3_t){{{21.60f, 0.6f, 116.70f}}}); // TESTING POSITION
    //set_camera_pos((shz_vec3_t){{{24.20f, 10.6f, 114.70f}}}); // BENCHMARK POSITION
    setup_tiles();
    return true;
}

void update(void)
{
    uint64_t start = perf_cntr_timer_ns();

    init_light((shz_vec3_t){{{0.0f, 0.0f, -1.0f}}});
    shz_xmtrx_init_identity();

    shz_vec4_t cam_pos = vec4_from_vec3f(get_camera_pos());
    shz_vec4_t cam_target = vec4_from_vec3f(get_camera_lookat_target());
    shz_vec4_t cam_up = vec4_from_vec3f(get_camera_up());

    mat_lookat((vector_t*)&cam_pos, (vector_t*)&cam_target, (vector_t*)&cam_up);
    shz_xmtrx_store_4x4((shz_mat4x4_t *)&v_mat);

    int num_objects = get_num_objects();

    int rows = 10;
    int cols = num_objects / 10;
    if(num_objects % 10 == 0){
        for (int object_index = 0; object_index < cols; object_index++) {
            float z = object_index * 5.5f;

            for (int i = 0; i < rows; i++) {
                object_t obj;
                int index = (object_index * rows) + i;

                if (get_object(index, &obj)) {
                    float x = i * 5.5f;
                    mesh_t *mesh = get_mesh(obj.id);
                    if (mesh) {
                        shz_vec3_t pos = vec3_new(x, obj.translation.y, z);
                        shz_vec3_t rot = get_obj_rotation(index);
                        rot.y += 0.01;
                        update_obj_rotation(index, &rot);
                        update_obj_translation(index, &pos);
                    }
                    process_graphics_pipeline_blocked(&obj);
                }
            }
        }
    }else{
        for(int i = 0; i < num_objects; i++){
            object_t obj;
            int index = i;

            if (get_object(index, &obj)) {
                float x = i * 5.5f;
                mesh_t *mesh = get_mesh(obj.id);
                if (mesh) {
                    shz_vec3_t pos = vec3_new(x, obj.translation.y, obj.translation.z);
                    shz_vec3_t rot = get_obj_rotation(index);
                    rot.y += 0.01;
                    rot.z += 0.02;
                    rot.x -= 0.02;
                    
                    update_obj_rotation(index, &rot);
                    update_obj_translation(index, &pos);
                }
                process_graphics_pipeline_blocked(&obj);
            }
            
        }
    }

    pipeline_total_ns += (perf_cntr_timer_ns() - start);
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
            draw_z_buffer = !draw_z_buffer;
        }
        if (PRESSED(CONT_B))
        {
            draw_mode++;
        }
        if (PRESSED(CONT_X))
        {
            draw_mode--;
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
            vel = shz_vec3_scale(vel, -0.2f);
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

        if (state->rtrig > 0)
        {
            shz_vec3_t pos = get_camera_pos();
            pos.y += state->rtrig * 0.001f;
            set_camera_pos(pos);
        }
        if (state->ltrig > 200)
        {
            shz_vec3_t pos = get_camera_pos();
            pos.y -= state->ltrig * 0.001f;
            set_camera_pos(pos);
        }

        if (draw_mode < 0)
        {
            draw_mode = 5;
        }
        else if (draw_mode > 5)
        {
            draw_mode = 0;
        }

        prev_buttons = buttons;
        prev_ltrig = state->ltrig;
        prev_rtrig = state->rtrig;
    }
}

void render(void)
{
    uint64_t start = perf_cntr_timer_ns();

    clear_z_buffer();

    /* for (int i = 0; i < num_skybox_triangles_to_render; i++) {
        triangle_t tri = skybox_triangles_to_render[i];
        draw_textured_triangle_scanline(&tri);
    } */
    // printf("Number of triangles to render: %d", num_triangles_to_render);
    for (int i = 0; i < num_triangles_to_render; i++)
    {
        triangle_t tri = triangles_to_render[i];
        switch (draw_mode) {
            case DRAW_WIRE:
                draw_triangle(&tri, 0xFFFF);
                break;

            case DRAW_FILL_TRIANGLE:
                draw_filled_triangle(&tri, 0xF800);
                break;

            case DRAW_FILL_TRIANGLE_WIRE:
                draw_filled_triangle_wire(&tri, 0xF800);
                break;

            case DRAW_TEXTURED:
                start_time = perf_cntr_timer_ns();
                /* draw_textured_triangle(&tri); */
                end_time = perf_cntr_timer_ns();
                break;

            case DRAW_TEXTURED_SCANLINE: {
                const texture_t *texture = get_texture(tri.id);
               // PROFILE_PERF_START(PROF_EVENT_DCACHE_FREEZE);
                // if (triangle_fully_inside_screen(&tri)) {

                    draw_textured_triangle_scanline(&tri, texture);
                // } else {
                //     draw_textured_triangle_scanline_fast(&tri, texture);
                // }
                dcache_freeze = PROFILE_PERF_STOP();

                break;
            }

            default:
                break;
        }
    }

    if (draw_z_buffer == true) {
        draw_z_buffer_to_screen();
    }

    skybox_tris_rendererd = num_triangles_to_render;
    draw_info(draw_mode, num_triangles_to_render, frame_count);
    num_triangles_to_render = 0;

    
}
static uint64_t app_start_ns = 0;
static uint64_t app_end_ns = 0;
int main(int argc, char *args[])
{
    printf("main entered\n");

    isRunning = initialize_window();
    setup();

    app_start_ns = perf_cntr_timer_ns();

    while (isRunning)
    {
         vid_flip(-1);
        
        draw_background_image();
        process_input();
        num_triangles_to_render = 0;

        update();
       
        bin_triangles(num_triangles_to_render);
        uint64_t start = perf_cntr_timer_ns();

        draw_tiles(draw_mode);
        render_total_ns += (perf_cntr_timer_ns() - start);

        /* render(); */
    #ifdef DEBUG
        uint64_t copy_start = perf_cntr_timer_ns();
    #endif


        sq_lock((void *)((uint8_t *)vram_s));
            shz_sq_memcpy32(SQ_MASK_DEST((void *)((uint8_t *)vram_s)),
                    (const void *)((uint8_t *)buffer),
                    buffer_size);

        sq_unlock();

        // shz_memcpy32((void *)((uint8_t *)vram_s),
        //      (const void *)((uint8_t *)buffer),
        //      buffer_size);

    #ifdef DEBUG
        copy_total_ns += (perf_cntr_timer_ns() - copy_start);
        frame_count++;
        if (frame_count == 1000) isRunning = false;
    #endif

   

    }
    #ifdef DEBUG
    app_end_ns = perf_cntr_timer_ns();

    uint64_t app_total_ns = app_end_ns - app_start_ns;
    double app_total_ms = (double)app_total_ns / 1e6;
    double avg_frame_ms = (frame_count > 0) ? (app_total_ms / (double)frame_count) : 0.0;
    double avg_fps = (app_total_ns > 0) ? ((double)frame_count * 1e9 / (double)app_total_ns) : 0.0;

    printf("render dcache freeze: %llu\n",
       (unsigned long long)dcache_freeze);

    printf("Average pipeline time: %.3f ms/frame\n",
           (frame_count > 0) ? ((double)pipeline_total_ns / (double)frame_count / 1e6) : 0.0);
    printf("Average render time: %.3f ms/frame\n",
           (frame_count > 0) ? ((double)render_total_ns / (double)frame_count / 1e6) : 0.0);
    printf("Average copy time: %.3f ms/frame\n",
           (frame_count > 0) ? ((double)copy_total_ns / (double)frame_count / 1e6) : 0.0);

    printf("ROM lifetime: %.3f ms total over %d frames\n", app_total_ms, frame_count);
    printf("Average total frame time: %.3f ms/frame\n", avg_frame_ms);
    printf("Average internal FPS: %.3f\n", avg_fps);

    double avg_pipeline_ms = (frame_count > 0) ? ((double)pipeline_total_ns / (double)frame_count / 1e6) : 0.0;
    double avg_render_ms   = (frame_count > 0) ? ((double)render_total_ns   / (double)frame_count / 1e6) : 0.0;
    double avg_copy_ms     = (frame_count > 0) ? ((double)copy_total_ns     / (double)frame_count / 1e6) : 0.0;
    double avg_other_ms    = avg_frame_ms - (avg_pipeline_ms + avg_render_ms + avg_copy_ms);

    printf("\n==== Final Summary ====\n");
    printf("Frames: %d\n", frame_count);
    printf("Total runtime: %.3f ms\n", app_total_ms);
    printf("Average FPS: %.3f\n", avg_fps);
    printf("Average frame time: %.3f ms\n", avg_frame_ms);
    printf("Breakdown: pipeline %.3f ms | render %.3f ms | copy %.3f ms | other %.3f ms\n",
           avg_pipeline_ms, avg_render_ms, avg_copy_ms, avg_other_ms);
    printf("Percentages: pipeline %.1f%% | render %.1f%% | copy %.1f%% | other %.1f%%\n",
           (avg_frame_ms > 0.0) ? (avg_pipeline_ms / avg_frame_ms) * 100.0 : 0.0,
           (avg_frame_ms > 0.0) ? (avg_render_ms   / avg_frame_ms) * 100.0 : 0.0,
           (avg_frame_ms > 0.0) ? (avg_copy_ms     / avg_frame_ms) * 100.0 : 0.0,
           (avg_frame_ms > 0.0) ? (avg_other_ms    / avg_frame_ms) * 100.0 : 0.0);

   
    print_pipeline_debug_stats(frame_count);
    #endif

    free_meshes();
    destroy_window();
    return 0;
}
