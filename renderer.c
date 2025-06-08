
#include <stdio.h>
#include "vector.h"
#include "display.h"
#include "mesh.h"

int frame_count = 0;
#define N_POINTS (9 * 9 * 9)
vec3_t cube_points[N_POINTS];
vec2_t projected_points[N_POINTS];
uint64_t start_time = 0, end_time = 0;
float fps = 0.0f;
float fov_factor = 640.0f;
vec3_t camera_position = {.x = 0, .y = 0, .z = -10};
vec3_t cube_rotation = {.x = 0, .y = 0, .z = 0};
int projected_point_count = 0;

void setup(){
    vid_set_mode(DM_640x480  | DM_MULTIBUFFER, PM_RGB565);
    vid_mode->fb_count = 2;
    int point_count = 0;
     for (float x = -1; x <= 1; x += 0.25) {
        for (float y = -1; y <= 1; y += 0.25) {
            for (float z = -1; z <= 1; z += 0.25) {
                vec3_t new_point = { .x = x, .y = y, .z = z };
               // new_point.z 
                cube_points[point_count++] = new_point;
            }
        }
    }
}

void project(vec3_t* point, vec2_t* result) {
    result->x = (fov_factor * point->x) / point->z;
    result->y = (fov_factor * point->y) / point->z;
}

void update(){
    maple_device_t *cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);

    if(cont) {
        cont_state_t *state = (cont_state_t *)maple_dev_status(cont);

        if(!state)
            return;

        if(state->a){
            cube_rotation.x = fmodf(cube_rotation.x - 0.01f, 2.0f * M_PI);
        }
        if(state->y){
            cube_rotation.x = fmodf(cube_rotation.x + 0.01f, 2.0f * M_PI);
        }
        if(state->b){
           cube_rotation.y = fmodf(cube_rotation.y + 0.01f, 2.0f * M_PI);
        }
        if(state->x){
           cube_rotation.y = fmodf(cube_rotation.y - 0.01f, 2.0f * M_PI);
        }
        if(state->buttons & CONT_DPAD_LEFT){
            camera_position.x -= 0.1;
        }
         if(state->buttons & CONT_DPAD_RIGHT){
            camera_position.x += 0.1;
        }
         if(state->buttons & CONT_DPAD_UP){
            camera_position.z += 0.1;
        }
         if(state->buttons & CONT_DPAD_DOWN){
            camera_position.z -= 0.1;
        }
    }

    // projected_point_count = 0;
    // for(int i = 0; i < N_POINTS; i++){
    //     vec3_t point = cube_points[i];
    //     vec3_rotate_y(&point, cube_rotation.y, &point);
    //     vec3_rotate_x(&point, cube_rotation.x, &point);
    //     point.x -= camera_position.x;
    //     point.y -= camera_position.y;
    //     point.z -= camera_position.z;
    //     if (point.z <= 0.1f) continue;
    //     projected_point_count++;
    //     vec2_t projected;
    //     project(&point, &projected);
    //     projected_points[i] = projected;
    // }
}


void render(){
    vid_clear(25, 25, 25);  // Clear i
    minifont_draw_str(vram_s + get_offset(20,10), 640, "Hello World, from Jamies Renderer!");
    // for(int i = 0; i < projected_point_count; i ++){
    //     vec2_t v = projected_points[i];
    //     vec3_t point = cube_points[i];
    //     uint8_t r = (uint8_t)(((point.x + 1.0f) / 2.0f) * 255.0f);
    //     uint8_t g = (uint8_t)(((point.y + 1.0f) / 2.0f) * 255.0f);
    //     uint8_t b = (uint8_t)(((point.z + 1.0f) / 2.0f) * 255.0f);

    //     // Convert to 0xRRGGBB
    //     uint32_t color = (r << 16) | (g << 8) | b;

    //     draw_rect(v.x + 320, v.y + 240, 4, 4, color);
    // }
}


int main(int argc, char **argv) {
    setup();
    while(1) {
        vid_waitvbl();
        update();
        render();
        vid_flip(frame_count%2);
        frame_count++;
    }

    return 0;
}
