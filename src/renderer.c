#include <stdio.h>
#include "../include/renderer.h"
#include "dc/syscalls.h"
#include <stdlib.h>
#include "shz_trig.h"
#include "shz_matrix.h"
#include "shz_xmtrx.h"
#include "shz_fpscr.h"
#include "../include/core.h"


bool isRunning = false;

int frame_count = 0;
int previous_frame_time = 0;
bool render_z_buffer = false;
int skybox_tris_rendererd = 0;

uint64_t triangle_time = 0;
uint64_t total_cycles = 0;
uint64_t elapsed_time = 0;

enum render_method render_mode;

void load_background_image(const char *path);
int get_num_objects(void);

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

    init_light((shz_vec3_t){{0.0f, 0.0f, -1.0f}});

    render_mode = RENDER_TEXTURED_SCANLINE;
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
    int skybox_id = load_assets("rd/Skybox.obj", "rd/SpeedHighway.png", vec3_new(1,1,1), vec3_new(0,0,0), vec3_new(0,0,0));
    (void)skybox_id;
    int cube_id = load_assets("rd/cube.obj", "rd/cube.png", vec3_new(1,1,1), vec3_new(0,0,0), vec3_new(0,0,0));

    for(int i = 0; i < 100; i++){
        create_object("cube", cube_id);
        printf("number of objects in scene: %d\n",get_num_objects());

    }

    set_camera_pos((shz_vec3_t){{24.20f, 10.6f, 114.70f}}); // TESTING POSITION
    // set_camera_pos((shz_vec3_t){0.6, 0.28, 6.6});
    return true;
}


void update(void)
{
    init_light((shz_vec3_t){{0.0f, 0.0f, -1.0f}});
    shz_xmtrx_init_identity();

    shz_vec4_t cam_pos = vec4_from_vec3f(get_camera_pos());
    shz_vec4_t cam_target = vec4_from_vec3f(get_camera_lookat_target());
    shz_vec4_t cam_up = vec4_from_vec3f(get_camera_up());

    // View matrix
    mat_lookat((vector_t*)&cam_pos,(vector_t*) &cam_target, (vector_t*)&cam_up);
    shz_xmtrx_store_4x4((shz_mat4x4_t *)&v_mat);
   

//    printf("skybox_pos %f, %f, %f\n", skybox->translation.x, skybox->translation.y, skybox->translation.z);
    
//    process_skybox();
    int num_objects = get_num_objects();
    int rows = 10;
    int cols = num_objects/10;
    // printf("Number of rows 10, number of columns: %d\n", cols);
    for (int object_index = 0; object_index < cols; object_index++) {
        float z = object_index * 5.5f;

        for (int i = 0; i < rows; i++) {
            object_t obj;
            int index = (object_index * rows) + i;

            if (get_object(index, &obj)) {
                float x = i * 5.5f;
                mesh_t *mesh = get_mesh(obj.id);
                mesh->translation = vec3_new(x, mesh->translation.y, z);
                process_graphics_pipeline(&obj);
            } else {
                printf("issue retrieving object at index: %d\n", index);
            }
        }
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

    /* for(int i = 0; i < num_skybox_triangles_to_render; i++){
        triangle_t tri =  skybox_triangles_to_render[i]; 
      //  start_time = perf_cntr_timer_ns();
          draw_textured_triangle_scanline(&tri);
        // end_time = perf_cntr_timer_ns();
        // avg += end_time - start_time;

   } */

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
                /* draw_textured_triangle(&tri); */
                end_time = perf_cntr_timer_ns();
                break;
            case RENDER_TEXTURED_SCANLINE: {
                const texture_t* texture = get_texture(tri.id);
                if(triangle_fully_inside_screen(&tri)){
                   // printf("triangle fully inside screen, using fast path\n");
                    draw_textured_triangle_scanline_fast(&tri, texture);

                }else{
                   // printf("triangle partially outside screen, using slow path\n");
                    draw_textured_triangle_scanline(&tri, texture);
                }
                // start_time = perf_cntr_timer_ns();
                // end_time = perf_cntr_timer_ns();
            break;
        }
        }
    }



    if(render_z_buffer == true){
        draw_z_buffer_to_screen();
    }
    skybox_tris_rendererd = num_triangles_to_render;
    //draw_info(render_mode, num_triangles_to_render, frame_count);
    num_triangles_to_render = 0; // Reset for next frame
    // num_skybox_triangles_to_render = 0; 

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
       
       // memset(z_buffer, 0, (WINDOW_WIDTH * WINDOW_HEIGHT) * sizeof(float));
        frame_count++;
       // if(frame_count == 1000) isRunning = false;
    }
    
    double avg_face_ns =
        (double)avg / ((double)frame_count * (double)skybox_tris_rendererd);

    printf("Skybox average: %.2f ns/face (%.3f ms/face)\n",
       avg_face_ns, avg_face_ns / 1e6);

    free_meshes();
    destroy_window();

    return 0;
}
