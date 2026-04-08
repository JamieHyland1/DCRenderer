#include "test_fixtures.h"
#include "unity.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "../include/triangle.h"
#include "../include/texture.h"
#include "test_utils.h"

static volatile uint32_t g_draw_sink = 0;

static inline int clampi_local(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static uint32_t sample_u16_buffer(const uint16_t *buf, int count) {
    uint32_t sum = 0;

    for (int i = 0; i < count; i += 32) {
        sum += buf[i];
    }

    if (count > 0) {
        sum += buf[count - 1];
    }

    return sum;
}

void test_bench_fullscreen_triangle_scanline(void) {
    reset_textures();

    texture_t texture = make_test_texture_4x4();
    int id = install_test_texture(texture);
    TEST_ASSERT_TRUE(id >= 0);

    triangle_t tri = make_fullscreen_triangle_1(id);

    const int iterations = 1000;

    uint64_t start = bench_now_ns();
    for (int i = 0; i < iterations; i++) {
        draw_textured_triangle_scanline(&tri, &texture);
    }
    uint64_t end = bench_now_ns();

    uint64_t elapsed = end - start;
    print_bench_result("triangle_scanline", elapsed, iterations, 0);

    TEST_ASSERT_TRUE(elapsed > 0);
}

void test_bench_fullscreen_quad_scanline(void) {
    reset_textures();

    texture_t texture = make_test_texture_4x4();
    int id = install_test_texture(texture);
    TEST_ASSERT_TRUE(id >= 0);

    triangle_t tri1 = make_fullscreen_triangle_1(id);
    triangle_t tri2 = make_fullscreen_triangle_2(id);

    const int iterations = 500;

    uint64_t start = bench_now_ns();
    for (int i = 0; i < iterations; i++) {
        draw_textured_triangle_scanline(&tri1, &texture);
        draw_textured_triangle_scanline(&tri2, &texture);
    }
    uint64_t end = bench_now_ns();

    uint64_t elapsed = end - start;
    print_bench_result("fullscreen_quad_scanline", elapsed, iterations, 0);

    TEST_ASSERT_TRUE(elapsed > 0);
}

void test_bench_clamp_only(void) {
    const int iterations = 1000000;
    const int sample_w = 256;
    const int sample_h = 256;

    float u = 13.25f;
    float v = 98.75f;
    float du_dx = 0.37f;
    float dv_dx = 0.21f;

    volatile int tx = 0;
    volatile int ty = 0;

    uint64_t start = bench_now_ns();
    for (int i = 0; i < iterations; i++) {
        tx = clampi_local((int)u, 0, sample_w - 1);
        ty = clampi_local((int)v, 0, sample_h - 1);
        u += du_dx;
        v -= dv_dx;
    }
    uint64_t end = bench_now_ns();

    g_draw_sink ^= (uint32_t)(tx + ty);
    print_bench_result("clamp_only", end - start, iterations, 0);

    TEST_ASSERT_TRUE((end - start) > 0);
}

void test_bench_fetch_only(void) {
    const int iterations = 1000000;
    const int shift = 8;

    static uint16_t tex_data[256 * 256];
    volatile uint16_t out = 0;

    int tx = 17;
    int ty = 42;

    uint64_t start = bench_now_ns();
    for (int i = 0; i < iterations; i++) {
        out = tex_data[(ty << shift) + tx];
        tx = (tx + 1) & 255;
        ty = (ty + 1) & 255;
    }
    uint64_t end = bench_now_ns();

    g_draw_sink ^= out;
    print_bench_result("fetch_only", end - start, iterations, 0);

    TEST_ASSERT_TRUE((end - start) > 0);
}

void test_bench_clamp_and_fetch(void) {
    const int iterations = 1000000;
    const int sample_w = 256;
    const int sample_h = 256;
    const int shift = 8;

    static uint16_t tex_data[256 * 256];
    volatile uint16_t out = 0;

    float u = 13.25f;
    float v = 98.75f;
    float du_dx = 0.37f;
    float dv_dx = 0.21f;

    uint64_t start = bench_now_ns();
    for (int i = 0; i < iterations; i++) {
        int tx = clampi_local((int)u, 0, sample_w - 1);
        int ty = clampi_local((int)v, 0, sample_h - 1);
        out = tex_data[(ty << shift) + tx];
        u += du_dx;
        v -= dv_dx;
    }
    uint64_t end = bench_now_ns();

    g_draw_sink ^= out;
    print_bench_result("clamp_and_fetch", end - start, iterations, 0);

    TEST_ASSERT_TRUE((end - start) > 0);
}

void test_bench_fetch_no_clamp(void) {
    const int iterations = 1000000;
    const int shift = 8;

    static uint16_t tex_data[256 * 256];
    volatile uint16_t out = 0;

    float u = 13.25f;
    float v = 98.75f;
    float du_dx = 0.37f;
    float dv_dx = 0.21f;

    uint64_t start = bench_now_ns();
    for (int i = 0; i < iterations; i++) {
        int tx = (int)u;
        int ty = (int)v;
        out = tex_data[(ty << shift) + tx];
        u += du_dx;
        v -= dv_dx;
    }
    uint64_t end = bench_now_ns();

    g_draw_sink ^= out;
    print_bench_result("fetch_no_clamp", end - start, iterations, 0);

    TEST_ASSERT_TRUE((end - start) > 0);
}

void test_bench_scanline_640_pixels_clamp(void) {
    const int iterations = 10000;
    const int count = 640;
    const int sample_w = 256;
    const int sample_h = 256;
    const int shift = 8;

    static uint16_t tex_data[256 * 256];
    static uint16_t dst[640];

    float u0 = 0.0f;
    float v0 = 128.0f;
    float du_dx = 0.40f;
    float dv_dx = 0.05f;

    uint64_t start = bench_now_ns();
    for (int it = 0; it < iterations; it++) {
        float u = u0;
        float v = v0;

        for (int i = 0; i < count; i++) {
            int tx = clampi_local((int)u, 0, sample_w - 1);
            int ty = clampi_local((int)v, 0, sample_h - 1);
            dst[i] = tex_data[(ty << shift) + tx];
            u += du_dx;
            v -= dv_dx;
        }
    }
    uint64_t end = bench_now_ns();

    g_draw_sink ^= sample_u16_buffer(dst, count);
    print_bench_result("scanline_640_clamp", end - start, iterations, 0);

    TEST_ASSERT_TRUE((end - start) > 0);
}

void test_bench_scanline_640_pixels_no_clamp(void) {
    const int iterations = 10000;
    const int count = 640;
    const int shift = 8;

    static uint16_t tex_data[256 * 256];
    static uint16_t dst[640];

    float u0 = 0.0f;
    float v0 = 128.0f;
    float du_dx = 0.10f;
    float dv_dx = 0.00f;

    uint64_t start = bench_now_ns();
    for (int it = 0; it < iterations; it++) {
        float u = u0;
        float v = v0;

        for (int i = 0; i < count; i++) {
            int tx = (int)u;
            int ty = (int)v;
            dst[i] = tex_data[(ty << shift) + tx];
            u += du_dx;
            v -= dv_dx;
        }
    }
    uint64_t end = bench_now_ns();

    g_draw_sink ^= sample_u16_buffer(dst, count);
    print_bench_result("scanline_640_no_clamp", end - start, iterations, 0);

    TEST_ASSERT_TRUE((end - start) > 0);
}