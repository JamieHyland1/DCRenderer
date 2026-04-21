#ifndef texture_h
#define texture_h
#include "upng.h"
#include <stdint.h>
#include <png/png.h>
typedef struct{
    float u;
    float v;
}tex2_t;

typedef struct {
    kos_img_t img;
    int cw, ch;
    int w_shift;
    int w_mask;
    int h_mask;
} texture_t;


tex2_t new_tex2(float u, float v);

bool texture_init(texture_t *t, kos_img_t img, int content_w, int content_h);
int get_num_textures();
texture_t* get_texture(int index);
int register_texture(texture_t tex);
void reset_textures(void);
bool install_texture_at(int index, texture_t tex);
#endif
