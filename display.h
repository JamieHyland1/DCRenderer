#ifndef display_h
#define display_h
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <kos.h>
#include <dc/video.h>
#include <dc/fmath.h>
#include <dc/maple.h>
#include <dc/biosfont.h>
#include <dc/maple/controller.h>
#include <utime.h>  // for timer_us_gettime64
#include <kos/thread.h>
#include <dc/minifont.h>

int  get_window_width(void);
int  get_window_height(void);

bool initialize_window(void);

void destroy_window(void);
int get_offset(int x, int y);
uint16_t rgb_to_rgb565(int r, int g, int b);
void render_color_buffer(void);
void draw_pixel(int x, int y, uint32_t color);
void draw_rect(int x, int y, int w, int h, uint32_t color);
void draw_grid(int xPlot, int rowSize);
void draw_boxed_grid(void);
void draw_line(int x1, int y1, int x2, int y2, uint32_t color);
void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
#endif