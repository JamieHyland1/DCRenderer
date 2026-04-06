#include "test_fixtures.h"

#include <string.h>

/*
 * Small 4x4 RGB565 test texture with obvious color pattern:
 *
 * R G B W
 * W B G R
 * G R W B
 * B W R G
 */
static uint16_t g_test_texture_4x4_pixels[16] = {
    0xF800, 0x07E0, 0x001F, 0xFFFF,
    0xFFFF, 0x001F, 0x07E0, 0xF800,
    0x07E0, 0xF800, 0xFFFF, 0x001F,
    0x001F, 0xFFFF, 0xF800, 0x07E0
};

static uint16_t g_test_texture_2x2_pixels[4] = {
    0xF800, 0x07E0,
    0x001F, 0xFFFF
};

texture_t make_test_texture_4x4(void) {
    texture_t tex;
    memset(&tex, 0, sizeof(tex));

    tex.img.data = g_test_texture_4x4_pixels;
    tex.img.w = 4;
    tex.img.h = 4;
    tex.cw = 4;
    tex.ch = 4;
    tex.w_shift = 2; /* log2(4) */

    return tex;
}

texture_t make_test_texture_2x2(void) {
    texture_t tex;
    memset(&tex, 0, sizeof(tex));

    tex.img.data = g_test_texture_2x2_pixels;
    tex.img.w = 2;
    tex.img.h = 2;
    tex.cw = 2;
    tex.ch = 2;
    tex.w_shift = 1; /* log2(2) */

    return tex;
}

triangle_t make_test_triangle_screen_space(int texture_id) {
    triangle_t tri;
    memset(&tri, 0, sizeof(tri));

    tri.points[0].x = 100.0f;
    tri.points[0].y = 100.0f;
    tri.points[0].z = 0.5f;
    tri.points[0].w = 1.0f;

    tri.points[1].x = 140.0f;
    tri.points[1].y = 100.0f;
    tri.points[1].z = 0.5f;
    tri.points[1].w = 1.0f;

    tri.points[2].x = 100.0f;
    tri.points[2].y = 140.0f;
    tri.points[2].z = 0.5f;
    tri.points[2].w = 1.0f;

    tri.texcoords[0].u = 0.0f;
    tri.texcoords[0].v = 0.0f;

    tri.texcoords[1].u = 1.0f;
    tri.texcoords[1].v = 0.0f;

    tri.texcoords[2].u = 0.0f;
    tri.texcoords[2].v = 1.0f;

    tri.normals[0].x = 0.0f;
    tri.normals[0].y = 0.0f;
    tri.normals[0].z = 1.0f;

    tri.normals[1].x = 0.0f;
    tri.normals[1].y = 0.0f;
    tri.normals[1].z = 1.0f;

    tri.normals[2].x = 0.0f;
    tri.normals[2].y = 0.0f;
    tri.normals[2].z = 1.0f;

    tri.id = texture_id;
    tri.orientation_from_light = 1.0f;

    return tri;
}

triangle_t make_test_triangle_object_space(int texture_id) {
    triangle_t tri;
    memset(&tri, 0, sizeof(tri));

    tri.points[0].x = 0.0f;
    tri.points[0].y = 0.0f;
    tri.points[0].z = 1.0f;
    tri.points[0].w = 1.0f;

    tri.points[1].x = 1.0f;
    tri.points[1].y = 0.0f;
    tri.points[1].z = 1.0f;
    tri.points[1].w = 1.0f;

    tri.points[2].x = 0.0f;
    tri.points[2].y = 1.0f;
    tri.points[2].z = 1.0f;
    tri.points[2].w = 1.0f;

    tri.texcoords[0].u = 0.0f;
    tri.texcoords[0].v = 0.0f;

    tri.texcoords[1].u = 1.0f;
    tri.texcoords[1].v = 0.0f;

    tri.texcoords[2].u = 0.0f;
    tri.texcoords[2].v = 1.0f;

    tri.normals[0].x = 0.0f;
    tri.normals[0].y = 0.0f;
    tri.normals[0].z = 1.0f;

    tri.normals[1].x = 0.0f;
    tri.normals[1].y = 0.0f;
    tri.normals[1].z = 1.0f;

    tri.normals[2].x = 0.0f;
    tri.normals[2].y = 0.0f;
    tri.normals[2].z = 1.0f;

    tri.id = texture_id;
    tri.orientation_from_light = 1.0f;

    return tri;
}

triangle_t make_test_triangle_custom(
    float x0, float y0, float z0, float w0,
    float x1, float y1, float z1, float w1,
    float x2, float y2, float z2, float w2,
    tex2_t uv0, tex2_t uv1, tex2_t uv2,
    int texture_id
) {
    triangle_t tri;
    memset(&tri, 0, sizeof(tri));

    tri.points[0].x = x0; tri.points[0].y = y0; tri.points[0].z = z0; tri.points[0].w = w0;
    tri.points[1].x = x1; tri.points[1].y = y1; tri.points[1].z = z1; tri.points[1].w = w1;
    tri.points[2].x = x2; tri.points[2].y = y2; tri.points[2].z = z2; tri.points[2].w = w2;

    tri.texcoords[0] = uv0;
    tri.texcoords[1] = uv1;
    tri.texcoords[2] = uv2;

    tri.normals[0].x = 0.0f; tri.normals[0].y = 0.0f; tri.normals[0].z = 1.0f;
    tri.normals[1].x = 0.0f; tri.normals[1].y = 0.0f; tri.normals[1].z = 1.0f;
    tri.normals[2].x = 0.0f; tri.normals[2].y = 0.0f; tri.normals[2].z = 1.0f;

    tri.id = texture_id;
    tri.orientation_from_light = 1.0f;

    return tri;
}

face_t make_test_face(void) {
    face_t f;
    memset(&f, 0, sizeof(f));

    f.a = 0;
    f.b = 1;
    f.c = 2;

    f.a_n = 0;
    f.b_n = 1;
    f.c_n = 2;

    f.a_uv.u = 0.0f; f.a_uv.v = 0.0f;
    f.b_uv.u = 1.0f; f.b_uv.v = 0.0f;
    f.c_uv.u = 0.0f; f.c_uv.v = 1.0f;

    return f;
}

void make_test_vertices_triangle(shz_vec3_t out_vertices[3]) {
    out_vertices[0].x = 0.0f; out_vertices[0].y = 0.0f; out_vertices[0].z = 1.0f;
    out_vertices[1].x = 1.0f; out_vertices[1].y = 0.0f; out_vertices[1].z = 1.0f;
    out_vertices[2].x = 0.0f; out_vertices[2].y = 1.0f; out_vertices[2].z = 1.0f;
}

void make_test_vertices_clockwise(shz_vec3_t out_vertices[3]) {
    out_vertices[0].x = 0.0f; out_vertices[0].y = 0.0f; out_vertices[0].z = 1.0f;
    out_vertices[1].x = 0.0f; out_vertices[1].y = 1.0f; out_vertices[1].z = 1.0f;
    out_vertices[2].x = 1.0f; out_vertices[2].y = 0.0f; out_vertices[2].z = 1.0f;
}

void make_test_vertices_counter_clockwise(shz_vec3_t out_vertices[3]) {
    out_vertices[0].x = 0.0f; out_vertices[0].y = 0.0f; out_vertices[0].z = 1.0f;
    out_vertices[1].x = 1.0f; out_vertices[1].y = 0.0f; out_vertices[1].z = 1.0f;
    out_vertices[2].x = 0.0f; out_vertices[2].y = 1.0f; out_vertices[2].z = 1.0f;
}

void make_test_vertices_degenerate(shz_vec3_t out_vertices[3]) {
    out_vertices[0].x = 0.0f; out_vertices[0].y = 0.0f; out_vertices[0].z = 1.0f;
    out_vertices[1].x = 1.0f; out_vertices[1].y = 1.0f; out_vertices[1].z = 1.0f;
    out_vertices[2].x = 2.0f; out_vertices[2].y = 2.0f; out_vertices[2].z = 1.0f;
}

shz_vec4_t make_test_vec4(float x, float y, float z, float w) {
    shz_vec4_t v;
    memset(&v, 0, sizeof(v));
    v.x = x; v.y = y; v.z = z; v.w = w;
    return v;
}

shz_vec3_t make_test_vec3(float x, float y, float z) {
    shz_vec3_t v;
    memset(&v, 0, sizeof(v));
    v.x = x; v.y = y; v.z = z;
    return v;
}

tex2_t make_test_tex2(float u, float v) {
    tex2_t t;
    memset(&t, 0, sizeof(t));
    t.u = u; t.v = v;
    return t;
}