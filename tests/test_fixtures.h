#ifndef TEST_FIXTURES_H
#define TEST_FIXTURES_H

#include "../include/triangle.h"
#include "../include/texture.h"

texture_t make_test_texture_4x4(void);
texture_t make_test_texture_2x2(void);
texture_t make_test_texture_4x4(void);

int install_test_texture(texture_t tex);
bool install_test_texture_at_id(int id, texture_t tex);

triangle_t make_test_triangle_screen_space(int texture_id);
triangle_t make_test_triangle_object_space(int texture_id);

triangle_t make_test_triangle_custom(
    float x0, float y0, float z0, float w0,
    float x1, float y1, float z1, float w1,
    float x2, float y2, float z2, float w2,
    tex2_t uv0, tex2_t uv1, tex2_t uv2,
    int texture_id
);

triangle_t make_fullscreen_triangle_1(int texture_id);
triangle_t make_fullscreen_triangle_2(int texture_id);

face_t make_test_face(void);

void make_test_vertices_triangle(shz_vec3_t out_vertices[3]);
void make_test_vertices_clockwise(shz_vec3_t out_vertices[3]);
void make_test_vertices_counter_clockwise(shz_vec3_t out_vertices[3]);
void make_test_vertices_degenerate(shz_vec3_t out_vertices[3]);

shz_vec4_t make_test_vec4(float x, float y, float z, float w);
shz_vec3_t make_test_vec3(float x, float y, float z);
tex2_t make_test_tex2(float u, float v);

#endif