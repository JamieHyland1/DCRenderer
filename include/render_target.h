#ifndef render_target_h
#define render_target_h

extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;

extern float* z_buffer;
extern uint16_t* buffer;
extern uint16_t* background_texture;
extern size_t buffer_size;

bool initialize_window(void);
void destroy_window(void);
bool init_test_render_buffers(size_t buffer_offset_bytes);
void shutdown_test_render_buffers(void);
void draw_background_image();
void draw_info(int render_mode, int num_triangles_to_render, int frame_count);
void draw_image(int dst_x, int dst_y, int width, int height, const uint16_t *pixels);
float get_z_buffer_at(int x, int y);
void update_zbuffer(int x, int y, float value);
void clear_z_buffer();
void draw_z_buffer_to_screen();
#endif
