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
    float avg_depth;
    uint16_t color;
    kos_img_t texture;
}triangle_t;

void draw_filled_triangle( 
    int x0, int y0, float z0, float w0,
    int x1, int y1, float z1, float w1,
    int x2, int y2, float z2, float w2, uint16_t color);


//TODO: DRAW VOID_DRAW_TEXTURED_TRIANGLE
void draw_textured_triangle(
    int x0, int y0, float z0, float w0, float u0, float v0,
    int x1, int y1, float z1, float w1, float u1, float v1,
    int x2, int y2, float z2, float w2, float u2, float v2,
    kos_img_t texture
);

vec3f_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p);
vec3f_t get_triangle_face_normal(vector_t vertices[3]);
void draw_filled_pixel(int x, int y,vector_t point_a, vector_t point_b, vector_t point_c,  uint16_t color);
void draw_textured_texel(int x, int y,vector_t point_a, vector_t point_b, vector_t point_c, float u0, float u1, float v0, float v1, float u2, float v2, kos_img_t texture);

#endif