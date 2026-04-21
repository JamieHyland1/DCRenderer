#include "../include/renderer.h"
#include <dc/minifont.h>
#include "shz_mem.h"
#include <stdlib.h>
// This file will contain the functions necessary to display our renderer to the screen

int get_offset(int x, int y){
     return y * WINDOW_WIDTH + x;
}

void drawRect(int x, int y, int w, int h, uint16_t color){
    for(int j = y; j <= y+h; j++){
        for(int i = x; i <= x+w; i++){
            draw_pixel(i,j,color);
        }
    }
}

inline void draw_pixel(int x, int y, uint16_t color){
    
    if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT) {
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
        draw_pixel((int)shz_roundf(x0), (int)shz_roundf(y0), color);
        return;
    }

    // Calculate increments
    float inv_steps = shz_invf_fsrra(steps);
    float x_inc = dx * inv_steps;
    float y_inc = dy * inv_steps;

    // Current position
    float x = x0;
    float y = y0;

    for (int i = 0; i <= (int)steps; i++) {
        // Draw the nearest pixel by rounding coordinates
        draw_pixel((int)shz_roundf(x), (int)shz_roundf(y), color);

        // Move to next position
        x += x_inc;
        y += y_inc;
    }
}