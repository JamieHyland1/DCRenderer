#ifndef display_h
#define display_h
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <kos.h>


extern bool isRunning;
int get_offset(int x, int y);
void drawRect(int x, int y, int w, int h, uint16_t color);
void draw_pixel(int x, int y, uint16_t color);
void draw_line(int x0, int y0, int x1, int y1, uint16_t color);
void draw_linef(float x0, float y0, float x1, float y1, uint16_t color);

#endif