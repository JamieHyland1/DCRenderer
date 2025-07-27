#ifndef display_h
#define display_h
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <kos.h>
#define FPS 60
#define FRAME_TARGET_TIME  (1000 / FPS)

int  get_window_width(void);
int  get_window_height(void);

bool initialize_window(void);

void draw_info(int render_mode, int num_triangles_to_render);
void destroy_window(void);
void clear_z_buffer();
void render_color_buffer(void);
void draw_pixel(int x, int y, uint16_t color);
void drawRect(int x, int y, int w, int h, uint16_t color);
void draw_line(int x1, int y1, int x2, int y2, uint16_t color);
void draw_image(int dst_x, int dst_y, int width, int height, const uint16_t *pixels);
float get_z_buffer_at(int x, int y);
void draw_z_buffer_to_screen(void);
void update_zbuffer(int x, int y, float value);
#endif