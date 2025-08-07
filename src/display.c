#include "../include/renderer.h"
#include <dc/minifont.h>
// This file will contain the functions necessary to display our renderer to the screen
// It will contain various SDL functions and functions related to drawing various primitive shapes

//declare an array to uint32 elements
float* z_buffer = NULL;

uint16_t* buffer = NULL;
size_t buffer_size = 0;
static uint16_t background_color = 0x6330;
int minWindowX = 640;
int minWindowY = 480;
int maxWindowX = 0;
int maxWindowY = 0;
static int window_width = 640;
static int window_height = 480;



int get_window_width(){
    return window_width;
}

int get_window_height(){
    return window_height;
}

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


#define PACK_PIXEL(r, g, b) ( ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3) )
void draw_pixel(int x, int y, uint16_t color){
    
    // if(x < minWindowX) minWindowX = x;
    // else if(x > maxWindowX) maxWindowX = x;
    // if(y < minWindowY) minWindowY = y;
    // else if(y > maxWindowY) maxWindowY = y;

    if (x < 0 || x >= 640 || y < 0 || y >= 480) {
        return;
    }
    buffer[get_offset(x, y)] = color;
}


void draw_line(int x0, int y0, int x1, int y1, uint16_t color) {
    int dx = fabs(x1 - x0);
    int dy = fabs(y1 - y0);

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

void draw_info(int render_mode, int num_triangles_to_render) {
    minifont_draw_str(buffer + get_offset(20, 10), 640, "Hello World, from Jamies Renderer!");
      // Log camera position, render mode, and triangle count on screen
    vec3f_t cam_pos = get_camera_pos();
    char cam_buf[64];
    snprintf(cam_buf, sizeof(cam_buf), "Camera: %.2f %.2f %.2f", cam_pos.x, cam_pos.y, cam_pos.z);
    minifont_draw_str(buffer + get_offset(20, 40), 640, cam_buf);

    char mode_buf[32];
    const char *mode_names[] = {
        "WIRE", "FILL_TRI", "FILL_TRI_WIRE", "TEXTURED", "TEXTURED_WIRE"
    };
    snprintf(mode_buf, sizeof(mode_buf), "Mode: %s", mode_names[render_mode]);
    minifont_draw_str(buffer + get_offset(20, 70), 640, mode_buf);

    char tri_buf[32];
    snprintf(tri_buf, sizeof(tri_buf), "Triangles: %d", num_triangles_to_render);
    minifont_draw_str(buffer + get_offset(20, 100), 640, tri_buf);
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


//tried unrolling sq_cpy_fast, dont think it worked

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

uint16_t get_background_color(){
    return background_color;
}