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

    int a_n;   // index of normal for vertex a
    int b_n;   // index of normal for vertex b
    int c_n;   // index of normal for vertex c
    
    tex2_t a_uv;
    tex2_t b_uv;
    tex2_t c_uv;
} face_t;

typedef struct {
    shz_vec4_t points[3];
    shz_vec3_t normals[3];
    tex2_t texcoords[3];
    kos_img_t texture;
    float orientation_from_light;
    float area;
}triangle_t;

void draw_triangle(const triangle_t* tri, uint16_t color);
void draw_filled_triangle(const triangle_t* tri, uint16_t color);
void draw_filled_triangle_wire(const triangle_t* tri, uint16_t color);
void draw_textured_triangle(const triangle_t *tri);
void draw_textured_triangle_bc_optimized(const triangle_t *tri);
void draw_textured_triangle_scanline(const triangle_t *tri);
void draw_flat_bottom_triangle(const triangle_t *tri);
void draw_flat_top_triangle(const triangle_t *tri);
shz_vec3_t get_triangle_face_normal(shz_vec4_t vertices[3]);
float edge_func(float x0, float y0, float x1, float y1, float x, float y);
#endif
