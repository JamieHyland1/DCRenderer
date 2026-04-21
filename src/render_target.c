#include "../include/renderer.h"
#include <dc/minifont.h>
#include "shz_mem.h"
#include <stdlib.h>
#include <string.h>

#define FPS 60
#define FRAME_TARGET_TIME  (1000 / FPS)
#define MAX_TRIANGLES_PER_MESH 10000

int WINDOW_WIDTH = 640;
int WINDOW_HEIGHT = 480;

float* z_buffer = NULL;
float *z_template = NULL;

uint16_t* buffer = NULL;
uint16_t* background_texture = NULL;
size_t buffer_size = 0;
static uint16_t background_color = 0x6330;

static void *buffer_base = NULL;
static void *background_base = NULL;


bool initialize_window(void){
    int count = WINDOW_WIDTH * WINDOW_HEIGHT;
    z_buffer    = (float*)aligned_alloc(32, sizeof(float) * WINDOW_WIDTH * WINDOW_HEIGHT);
    z_template  = (float *)aligned_alloc(32, sizeof(float) * WINDOW_WIDTH * WINDOW_HEIGHT);

    for (size_t i = 0; i < count; i++) {
        z_buffer[i] = 1.0f;
        z_template[i] = 0.0f;
    }

    return true;
}

void destroy_window(void){
    free(z_buffer);
    free(z_template);
}

bool init_test_render_buffers(size_t buffer_offset_bytes) {
    buffer_size = WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint16_t);

    /* keep the render buffer 32-byte aligned */
    if ((buffer_offset_bytes & 31) != 0) {
        return false;
    }

    /* free old allocations so repeated calls can test different offsets */
    free(buffer_base);
    free(background_base);
    free(z_buffer);
    free(z_template);

    buffer_base = NULL;
    background_base = NULL;
    buffer = NULL;
    background_texture = NULL;
    z_buffer = NULL;
    z_template = NULL;

    /* allocate extra room so we can shift buffer forward */
    buffer_base = aligned_alloc(32, buffer_size + buffer_offset_bytes);
    if (!buffer_base) return false;

    background_base = aligned_alloc(32, buffer_size);
    if (!background_base) {
        free(buffer_base);
        buffer_base = NULL;
        return false;
    }

    z_buffer = (float *)aligned_alloc(32, sizeof(float) * WINDOW_WIDTH * WINDOW_HEIGHT);
    if (!z_buffer) {
        free(buffer_base);
        free(background_base);
        buffer_base = NULL;
        background_base = NULL;
        return false;
    }

    z_template = (float *)aligned_alloc(32, sizeof(float) * WINDOW_WIDTH * WINDOW_HEIGHT);
    if (!z_template) {
        free(buffer_base);
        free(background_base);
        free(z_buffer);
        buffer_base = NULL;
        background_base = NULL;
        z_buffer = NULL;
        return false;
    }

    buffer = (uint16_t *)((uint8_t *)buffer_base + buffer_offset_bytes);
    background_texture = (uint16_t *)background_base;

    memset(buffer, 0, buffer_size);
    memset(background_texture, 0, buffer_size);

    for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; i++) {
        z_buffer[i] = 1.0f;
        z_template[i] = 0.0f;
    }

    return true;
}

void shutdown_test_render_buffers(void) {
    free(buffer_base);
    free(background_base);
    free(z_buffer);
    free(z_template);

    buffer_base = NULL;
    background_base = NULL;
    buffer = NULL;
    background_texture = NULL;
    z_buffer = NULL;
    z_template = NULL;
    buffer_size = 0;
}


float get_z_buffer_at(int x, int y){
    if((x >= 640  || x < 0) || (y >= 480 || y < 0)){
        //printf("zbuffer coordinates: %d, %d\n",x,y);
        return 0.0;
    } 
    return z_buffer[(640 * y) + x];
}

void update_zbuffer(int x, int y, float value){
    if(value < 0.0 || value > 1.0  || x < 0  || x >= 640  || y < 0  || y >= 480){
        return;
    }
    
    z_buffer[(640 * y) + x] = value;
}

void clear_z_buffer(){
    memcpy(z_buffer, z_template, sizeof(float) * WINDOW_WIDTH * WINDOW_HEIGHT);
}

void draw_background_image(){
    // uint64_t start_time = perf_cntr_timer_ns();   
    memcpy(buffer, background_texture, buffer_size);
    // sq_cpy((void *)((uint8_t *)buffer), (const void *)((uint8_t *)background_texture), 640 * 480 * sizeof(uint16_t));
    // uint64_t end_time = perf_cntr_timer_ns();
    // uint64_t elapsed_time = end_time - start_time;
    // printf("Background draw time: %llu ns\n", elapsed_time);

}

void load_background_image(const char* path){
    kos_img_t img;
    png_to_img(path, 0, &img);

    if (img.w <= 0 || img.h <= 0 || !img.data) {
        fprintf(stderr, "Error loading PNG file: %s\n", path);
        return;
    }

    const uint16_t *src = (const uint16_t *)img.data;
    const int src_w = img.w;
    const int src_h = img.h;

    const int dst_w = WINDOW_WIDTH;
    const int dst_h = WINDOW_HEIGHT;

    // Scale up enough to cover the whole screen, then crop center.
    float scale_x = (float)dst_w / (float)src_w;
    float scale_y = (float)dst_h / (float)src_h;
    float scale = (scale_x > scale_y) ? scale_x : scale_y;

    int scaled_w = (int)(src_w * scale);
    int scaled_h = (int)(src_h * scale);

    // Center-crop offsets in scaled space.
    int crop_x = (scaled_w - dst_w) / 2;
    int crop_y = (scaled_h - dst_h) / 2;

    for (int y = 0; y < dst_h; y++) {
        // Map destination y back into source y.
        int sy = (int)(((y + crop_y) / scale));
        if (sy < 0) sy = 0;
        if (sy >= src_h) sy = src_h - 1;

        for (int x = 0; x < dst_w; x++) {
            int sx = (int)(((x + crop_x) / scale));
            if (sx < 0) sx = 0;
            if (sx >= src_w) sx = src_w - 1;

            background_texture[y * dst_w + x] = src[sy * src_w + sx];
        }
    }
}

uint16_t get_background_color(){
    return background_color;
}

void draw_z_buffer_to_screen(void) {
    // Find min/max excluding sentinel cleared values
    const float sentinel = -1e8f;
    float zmin = 1e9f, zmax = -1e9f;
    for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; ++i) {
        float z = z_buffer[i];
        if (z <= sentinel) continue;
        if (z < zmin) zmin = z;
        if (z > zmax) zmax = z;
    }

    if (zmin == 1e9f && zmax == -1e9f) {
        // nothing to draw, fill with background
        for (int i = 0; i < WINDOW_WIDTH * WINDOW_HEIGHT; ++i) buffer[i] = get_background_color();
        return;
    }

    float range = zmax - zmin;
    if (range < 1e-6f) range = 1.0f;

    for (int y = 0; y < WINDOW_HEIGHT; y++) {
        for (int x = 0; x < WINDOW_WIDTH; x++) {
            float z = z_buffer[y * WINDOW_WIDTH + x];
            if (z <= sentinel) {
                buffer[get_offset(x, y)] = get_background_color();
                continue;
            }
            float norm_z = (z - zmin) / range; // normalize to 0..1
            if (norm_z < 0.0f) norm_z = 0.0f;
            if (norm_z > 1.0f) norm_z = 1.0f;
            uint8_t intensity = (uint8_t)(norm_z * 255.0f);
            uint16_t color = ((intensity >> 3) << 11) | ((intensity >> 2) << 5) | (intensity >> 3);
            buffer[get_offset(x, y)] = color;
        }
    }
}

void draw_info(int render_mode, int num_triangles_to_render, int frame_count) {
    minifont_draw_str(buffer + get_offset(20, 10), WINDOW_WIDTH, "Hello World, from Jamies Renderer!");
      // Log camera position, render mode, and triangle count on screen
    shz_vec3_t cam_pos = get_camera_pos();
    char cam_buf[64];
    snprintf(cam_buf, sizeof(cam_buf), "Camera: %.2f %.2f %.2f", cam_pos.x, cam_pos.y, cam_pos.z);
    minifont_draw_str(buffer + get_offset(20, 40), WINDOW_WIDTH, cam_buf);

    char mode_buf[32];
    const char *mode_names[] = {
        "WIRE", "FILL_TRI", "FILL_TRI_WIRE", "TEXTURED", "TEXTURED_OPTIMIZED"
    };
    snprintf(mode_buf, sizeof(mode_buf), "Mode: %s", mode_names[render_mode]);
    minifont_draw_str(buffer + get_offset(20, 70), WINDOW_WIDTH, mode_buf);

    char tri_buf[32];
    snprintf(tri_buf, sizeof(tri_buf), "Triangles: %d", num_triangles_to_render);
    minifont_draw_str(buffer + get_offset(20, 100), WINDOW_WIDTH, tri_buf);

    char frame_buf[32];
    snprintf(frame_buf, sizeof(frame_buf), "Frame: %d", frame_count);
    minifont_draw_str(buffer + get_offset(20, 130), WINDOW_WIDTH, frame_buf);
}

void draw_image(int dst_x, int dst_y, int width, int height, const uint16_t *pixels) {
    for (int y = 0; y < height; y++) {
        int screen_y = dst_y + y;
        for (int x = 0; x < width; x++) {
            int screen_x = dst_x + x;
            uint16_t color = pixels[y * width + x];
            if(color <= 0) { // Skip transparent pixels
                continue;
            }
            draw_pixel(screen_x, screen_y, color);
        }
    }
}
