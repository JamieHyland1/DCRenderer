#include "display.h"
#include <math.h>


static int window_width = 480;
static int window_height = 640;

int get_window_width(){
    return window_width;
}

int get_window_height(){
    return window_height;
}

bool initialize_window(void){
    return true;
}

int get_offset(int x, int y){
     return y * 640 + x;
}

uint16_t rgb_to_rgb565(int r, int g, int b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

#define PACK_PIXEL(r, g, b) ( ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3) )
void draw_pixel(int x, int y, uint32_t color){
    if (x < 0 || x >= 640 || y < 0 || y >= 480) {
     return;
    }
    uint8_t r = (color >> 16) & 0xFF;  // Extract red
    uint8_t g = (color >> 8)  & 0xFF;  // Extract green
    uint8_t b =  color        & 0xFF;  // Extract blue
    vram_s[get_offset(x,y)]  = PACK_PIXEL(r, g, b);
}

int lerp(float a, float b, float t){
    return (int) a + t * (b - a);
}


void draw_rect(int x, int y, int w, int h, uint32_t color){
    for(int j = y; j <= y+h; j++){
        for(int i = x; i <= x+w; i++){
            draw_pixel(i,j,color);
        }
    }
}

void draw_boxed_grid(void){
    for(int y = 0; y < window_height; y++){
        for(int x = 0; x < window_width; x++){
            if(x%40 == 0  || y%40 == 0)draw_pixel(x,y,0xffffff);
        }
    }
}

void draw_grid(int xPlot, int rowSize){
    for(int y = 0; y < window_height; y+=10){
        for(int x = 0; x < window_width; x++){
            int index = (window_width * y) + x;
            if(x%10 == 0){
               // vram_s[index] = 0xffffff;
            }
        }
    }
}

void draw_line(int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = fabs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -fabs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        draw_pixel(x0, y0, color);

        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;

        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }

        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}


void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color);
}

void swap_int(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Helper to draw a horizontal line
void draw_hline(int x1, int x2, int y, uint32_t color) {
    if (x1 > x2) swap_int(&x1, &x2);
    for (int x = x1; x <= x2; x++) {
        draw_pixel(x, y, color);
    }
}

void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    // Sort points by y-coordinate ascending (y0 <= y1 <= y2)
    if (y0 > y1) { swap_int(&y0, &y1); swap_int(&x0, &x1); }
    if (y0 > y2) { swap_int(&y0, &y2); swap_int(&x0, &x2); }
    if (y1 > y2) { swap_int(&y1, &y2); swap_int(&x1, &x2); }

    int total_height = y2 - y0;
    for (int i = 0; i < total_height; i++) {
        bool second_half = i > (y1 - y0) || y1 == y0;
        int segment_height = second_half ? (y2 - y1) : (y1 - y0);
        float alpha = (float)i / total_height;
        float beta  = (float)(i - (second_half ? y1 - y0 : 0)) / segment_height;

        int ax = x0 + (int)((x2 - x0) * alpha);
        int bx = second_half
               ? x1 + (int)((x2 - x1) * beta)
               : x0 + (int)((x1 - x0) * beta);

        int y = y0 + i;
        draw_hline(ax, bx, y, color);
    }
}

void destroy_window(void){
  
}