#ifndef triangle_h
#define triangle_h
#include "vector.h"
#include "texture.h"
#include <stdint.h>
#include <stdio.h>
#include "upng.h"
#include <png/png.h>
#include <dc/vector.h>
typedef struct {
    int a;
    int b;
    int c;
    tex2_t a_uv;
    tex2_t b_uv;
    tex2_t c_uv;
} face_t;

typedef struct {
    vector_t points[3];
    tex2_t texcoords[3];
    kos_img_t texture;
}triangle_t;

void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t color);
void draw_filled_triangle(vec2i_t* v0, vec2i_t* v1, vec2i_t* v2, uint16_t color);
void draw_filled_triangle_wire(vec2i_t* v0, vec2i_t* v1, vec2i_t* v2, uint16_t color);
void draw_textured_triangle(vec2i_t* v0, vec2i_t* v1, vec2i_t* v2, vec2_t* uv0, vec2_t* uv1, vec2_t* uv2, kos_img_t texture);
void draw_textured_triangle_new(vec2i_t* v0, vec2i_t* v1, vec2i_t* v2, vec2_t* uv0, vec2_t* uv1, vec2_t* uv2, kos_img_t texture); // just for before/after
vec3f_t get_triangle_face_normal(vector_t vertices[3]);

#endif