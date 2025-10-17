#include "../include/renderer.h"
#include <dc/minifont.h>
#include "shz_mem.h"
#include <stdlib.h>
// This file will contain the functions necessary to display our renderer to the screen
// It will contain various SDL functions and functions related to drawing various primitive shapes
int WINDOW_WIDTH = 640;
int WINDOW_HEIGHT = 480;
//declare an array to uint32 elements
float* z_buffer = NULL;
bool isRunning = false;
uint16_t* buffer = NULL;
uint16_t* background_texture = NULL;
size_t buffer_size = 0;
static uint16_t background_color = 0x6330;
int minWindowX = 640;
int minWindowY = 480;
int maxWindowX = 0;
int maxWindowY = 0;
static int window_width = 640;
static int window_height = 480;

int get_offset(int x, int y){
     return y * 640 + x;
}

void draw_z_buffer_to_screen(void) {
    for (int y = 0; y < window_height; y++) {
        for (int x = 0; x < window_width; x++) {
            float z = z_buffer[y * window_width + x];
            // Map z (0.0 - 1.0) to grayscale (0x0000 - 0xFFFF)
            uint8_t intensity = (uint8_t)(z * 255.0f);
            // RGB565 grayscale
            uint16_t color = ((intensity >> 3) << 11) | ((intensity >> 2) << 5) | (intensity >> 3);
            buffer[get_offset(x, y)] = color;
        }
    }
}

void drawRect(int x, int y, int w, int h, uint16_t color){
    for(int j = y; j <= y+h; j++){
        for(int i = x; i <= x+w; i++){
            draw_pixel(i,j,color);
        }
    }
}

inline void draw_pixel(int x, int y, uint16_t color){
    
    if (x < 0 || x >= 640 || y < 0 || y >= 480) {
        return;
    }
    buffer[get_offset(x, y)] = color;
}

void draw_line(int x0, int y0, int x1, int y1, uint16_t color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx - dy;

    while (1) {
        draw_pixel(x0, y0, color);

        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;

        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void draw_linef(float x0, float y0, float x1, float y1, uint16_t color) {
    // Calculate differences and steps
    float dx = x1 - x0;
    float dy = y1 - y0;
    float steps = fmaxf(fabsf(dx), fabsf(dy));

    if (steps == 0) {
        // Single point case
        draw_pixel((int)roundf(x0), (int)roundf(y0), color);
        return;
    }

    // Calculate increments
    float x_inc = dx / steps;
    float y_inc = dy / steps;

    // Current position
    float x = x0;
    float y = y0;

    for (int i = 0; i <= (int)steps; i++) {
        // Draw the nearest pixel by rounding coordinates
        draw_pixel((int)roundf(x), (int)roundf(y), color);

        // Move to next position
        x += x_inc;
        y += y_inc;
    }
}

void destroy_window(void){
    free(z_buffer);
}

bool initialize_window(void){

    z_buffer = (float*)malloc(sizeof(float) * window_width * window_height);

    return true;
}

void clear_z_buffer(){
    for(int i = 0; i < 640 * 480; i++){
        z_buffer[i] = 0.0;
    }
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

void draw_info(int render_mode, int num_triangles_to_render, int frame_count) {
    minifont_draw_str(buffer + get_offset(20, 10), 640, "Hello World, from Jamies Renderer!");
      // Log camera position, render mode, and triangle count on screen
    shz_vec3_t cam_pos = get_camera_pos();
    char cam_buf[64];
    snprintf(cam_buf, sizeof(cam_buf), "Camera: %.2f %.2f %.2f", cam_pos.x, cam_pos.y, cam_pos.z);
    minifont_draw_str(buffer + get_offset(20, 40), 640, cam_buf);

    char mode_buf[32];
    const char *mode_names[] = {
        "WIRE", "FILL_TRI", "FILL_TRI_WIRE", "TEXTURED", "TEXTURED_OPTIMIZED"
    };
    snprintf(mode_buf, sizeof(mode_buf), "Mode: %s", mode_names[render_mode]);
    minifont_draw_str(buffer + get_offset(20, 70), 640, mode_buf);

    char tri_buf[32];
    snprintf(tri_buf, sizeof(tri_buf), "Triangles: %d", num_triangles_to_render);
    minifont_draw_str(buffer + get_offset(20, 100), 640, tri_buf);

    char frame_buf[32];
    snprintf(frame_buf, sizeof(frame_buf), "Frame: %d", frame_count);
    minifont_draw_str(buffer + get_offset(20, 130), 640, frame_buf);
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

void copy_back_buffer(uint16_t* buffer){
   size_t size = 32;
   sq_lock((void*)((uint8_t*)vram_s));
    for (size_t i = 0; i < buffer_size; i += size * 4) {
        void* dest0 = ((uintptr_t)vram_s + i);
        const void* src0 = (const void*)((uint8_t*)buffer + i);
        void* dest1 = ((uintptr_t)vram_s + i + size);
        const void* src1 = (const void*)((uint8_t*)buffer + i + size);
        void* dest2 = ((uintptr_t)vram_s + i + size * 2);
        const void* src2 = (const void*)((uint8_t*)buffer + i + size * 2);
        void* dest3 = ((uintptr_t)vram_s + i + size * 3);
        const void* src3 = (const void*)((uint8_t*)buffer + i + size * 3);

        sq_fast_cpy(dest0, src0, 1);
        sq_flush(dest0);
        sq_wait();
        
        sq_fast_cpy(dest1, src1, 1);
        sq_flush(dest1);
        sq_wait();
        
        sq_fast_cpy(dest2, src2, 1);
        sq_flush(dest2);
        sq_wait();
        
        sq_fast_cpy(dest3, src3, 1);
        sq_flush(dest3);
        sq_wait();
    }
    
    sq_unlock();
}

void set_background_color(uint16_t color){
    background_color = color;
}

void load_background_image(const char* path){
    kos_img_t img;
    png_to_img(path, 0, &img);
    if(img.w > 0){
        uint16_t *tex_data = (uint16_t *)img.data;
   
        memcpy(background_texture, tex_data, 640 * 480 * sizeof(uint16_t));
    }else{
         fprintf(stderr, "Error loading PNG file: %s\n", path);
        return;
    }
   
}

void draw_background_image(){
    // uint64_t start_time = perf_cntr_timer_ns();   
    shz_memcpy32(buffer, background_texture, buffer_size);
    // sq_cpy((void *)((uint8_t *)buffer), (const void *)((uint8_t *)background_texture), 640 * 480 * sizeof(uint16_t));
    // uint64_t end_time = perf_cntr_timer_ns();
    // uint64_t elapsed_time = end_time - start_time;
    // printf("Background draw time: %llu ns\n", elapsed_time);

}

uint16_t get_background_color(){
    return background_color;
}
