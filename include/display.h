#ifndef display_h
#define display_h
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <kos.h>
#define FPS 60
#define FRAME_TARGET_TIME  (1000 / FPS)
extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;
extern bool isRunning;

int  get_window_width(void);
int  get_window_height(void);
extern float* z_buffer;
extern uint16_t* buffer;
extern uint16_t* background_texture;
extern size_t buffer_size;
extern int minWindowX;
extern int minWindowY;
extern int maxWindowX;
extern int maxWindowY;

int get_offset(int x, int y);
void draw_z_buffer_to_screen(void);
void drawRect(int x, int y, int w, int h, uint16_t color);
void draw_pixel(int x, int y, uint16_t color);
void draw_line(int x1, int y1, int x2, int y2, uint16_t color);
void draw_linef(float x0, float y0, float x1, float y1, uint16_t color);
void destroy_window(void);
bool initialize_window(void);
void clear_z_buffer();
float get_z_buffer_at(int x, int y);
void update_zbuffer(int x, int y, float value);
void draw_info(int render_mode, int num_triangles_to_render, int frame_count);
void draw_image(int dst_x, int dst_y, int width, int height, const uint16_t *pixels);
void copy_back_buffer(uint16_t* buffer);
void copy_back_buffer_old(uint16_t* buffer);
uint16_t get_background_color();
void set_background_color(uint16_t color);
void load_background_image(const char* path);
void draw_background_image();
#endif