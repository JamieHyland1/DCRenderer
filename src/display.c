#include <math.h>
#include <stdlib.h>
#include <dc/minifont.h>
#include "../include/display.h"
#include "../include/camera.h"
#include "../include/vector.h"
#include "../include/matrix.h"
#include "../include/triangle.h"
#include "../include/pipeline.h"

//////////////////////////
// Handles the display of the renderer
// This includes drawing pixels, lines, rectangles, and images to the screen
// It also handles the z-buffer for depth testing
////////////////////////////////////////////////////////////////////////////

float* z_buffer = NULL;
int window_width = 640;
int window_height = 480;

int get_window_width(){
    return window_width;
}

int get_window_height(){
    return window_height;
}

int get_offset(int x, int y){
     return y * 640 + x;
}

bool initialize_window(void){

    z_buffer = (float*)malloc(sizeof(float) * window_width * window_height);

    return true;
}

void clear_z_buffer(){
    for(int i = 0; i < window_width * window_height; i++){
        z_buffer[i] = 0.0;
    }
}

void draw_z_buffer_to_screen(void) {
    for (int y = 0; y < window_height; y++) {
        for (int x = 0; x < window_width; x++) {
            float z = z_buffer[y * window_width + x];
            // Map z (0.0 - 1.0) to grayscale (0x0000 - 0xFFFF)
            uint8_t intensity = (uint8_t)(z * 255.0f);
            // RGB565 grayscale
            uint16_t color = ((intensity >> 3) << 11) | ((intensity >> 2) << 5) | (intensity >> 3);
            vram_s[get_offset(x, y)] = color;
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
    if (x < 0 || x >= 640 || y < 0 || y >= 480) {
        return;
    }
    vram_s[get_offset(x, y)] = color;
}

void draw_line(int x0, int y0, int x1, int y1, uint16_t color) {
    int delta_x = (x1 - x0);
    int delta_y = (y1 - y0);

    int longest_side_length = (fabsf(delta_x) >= fabsf(delta_y)) ? fabsf(delta_x) : fabsf(delta_y);

    float x_inc = delta_x / (float)longest_side_length; 
    float y_inc = delta_y / (float)longest_side_length;

    float current_x = x0;
    float current_y = y0;
    for (int i = 0; i <= longest_side_length; i++) {
       draw_pixel(round(current_x), round(current_y), color);
        current_x += x_inc;
        current_y += y_inc;
    }
}

void destroy_window(void){
    free(z_buffer);
}

float get_z_buffer_at(int x, int y){
    if((x >= window_width  || x < 0) || (y >= window_height || y < 0)){
        //printf("zbuffer coordinates: %d, %d\n",x,y);
        return 0.0;
    } 
    return z_buffer[(window_width * y) + x];
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

void update_zbuffer(int x, int y, float value){
    if(value < 0.0 || value > 1.0  || x < 0  || x >= window_width  || y < 0  || y >= window_height){
        return;
    }
    
    z_buffer[(window_width * y) + x] = value;
}

void draw_info(int render_mode, int num_triangles_to_render) {
    minifont_draw_str(vram_s + get_offset(20, 10), 640, "Hello World, from Jamies Renderer!");
      // Log camera position, render mode, and triangle count on screen
    vec3f_t cam_pos = get_camera_pos();
    char cam_buf[64];
    snprintf(cam_buf, sizeof(cam_buf), "Camera: %.2f %.2f %.2f", cam_pos.x, cam_pos.y, cam_pos.z);
    minifont_draw_str(vram_s + get_offset(20, 40), 640, cam_buf);

    char mode_buf[32];
    const char *mode_names[] = {
        "WIRE", "FILL_TRI", "FILL_TRI_WIRE", "TEXTURED", "TEXTURED_WIRE"
    };
    snprintf(mode_buf, sizeof(mode_buf), "Mode: %s", mode_names[render_mode]);
    minifont_draw_str(vram_s + get_offset(20, 70), 640, mode_buf);

    char tri_buf[32];
    snprintf(tri_buf, sizeof(tri_buf), "Triangles: %d", num_triangles_to_render);
    minifont_draw_str(vram_s + get_offset(20, 100), 640, tri_buf);
}

void render(void)
{  
    vid_clear(25, 25, 25);
    clear_z_buffer();
    
    for(int i = 0; i < num_triangles_to_render; i ++){
        triangle_t tri = triangles_to_render[i];
        switch(render_mode)
        {
            case RENDER_WIRE:
                draw_triangle(
                    tri.points[0].x, tri.points[0].y, // vertex A
                    tri.points[1].x, tri.points[1].y, // vertex B
                    tri.points[2].x, tri.points[2].y, // vertex C
                    0xFFFF // color (16-bit white)
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
                    &(vec2_t){   tri.texcoords[0].u,   tri.texcoords[0].v},
                    &(vec2_t){   tri.texcoords[1].u,   tri.texcoords[1].v},
                    &(vec2_t){   tri.texcoords[2].u,   tri.texcoords[2].v},
                    tri.texture
                );
                break;
            case RENDER_TEXTURED_WIRE:
                // TODO reimplement this later
                break;
        }

        draw_info(render_mode, num_triangles_to_render);

    }
}
