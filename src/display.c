#include "../include/display.h"
#include <math.h>
#include <stdlib.h>
// This file will contain the functions necessary to display our renderer to the screen
// It will contain various SDL functions and functions related to drawing various primitive shapes

//declare an array to uint32 elements
static float* z_buffer = NULL;

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

bool initialize_window(void){
    
    // if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
    //     fprintf(stderr, "Error initializing SDL window");
    //     return false;
    // }

    // SDL_DisplayMode displayMode;

    // SDL_GetCurrentDisplayMode(0, &displayMode);

    //TODO create SDL window
    // window = SDL_CreateWindow(
    //     "The window into jamie's madness",
    //     SDL_WINDOWPOS_CENTERED,
    //     SDL_WINDOWPOS_CENTERED,
    //     full_screen_width,
    //     full_screen_height,
    //     SDL_WINDOW_FOREIGN
    // );


    //TODO create SDL Renderer
    // renderer = SDL_CreateRenderer(window,-1, 0);

    // if(!renderer){
    //     fprintf(stderr, "Error creating SDL renderer");
    //     return false;
    // }

    // SDL_SetWindowBordered(window,SDL_WINDOWPOS_CENTERED);


    // Allocating memory for color buffer and Z buffer
    // color_buffer = (uint16_t*) malloc(sizeof(uint16_t) * window_width * window_height);
    z_buffer = (float*)malloc(sizeof(float) * window_width * window_height);
    
    // color_buffer_texture = SDL_CreateTexture(
    //     renderer,
    //     SDL_PIXELFORMAT_RGBA32,
    //     SDL_TEXTUREACCESS_STREAMING,
    //     window_width,
    //     window_height
    // );


    //  if(!color_buffer){
    //     fprintf(stderr, "couldnt allocate memory for color buffer");
    //     return false;
        
    // }

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

    int longest_side_length = (fabs(delta_x) >= fabs(delta_y)) ? fabs(delta_x) : fabs(delta_y);

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

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color) {
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color);
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
