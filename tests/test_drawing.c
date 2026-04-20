#include "test_fixtures.h"
#include "test_drawing.h"
#include "unity.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <dc/perfctr.h>
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

static void print_counter_result(const char *name, uint64_t total_ns, uint64_t counter_value, int iterations) {
    const double elapsed_us = (double)total_ns / 1000.0;

    print_bench_result(name, total_ns, iterations, 0);
    printf("%s: PMCR_PIPELINE_FREEZE_BY_DCACHE_MISS_MODE total=%llu, per iter=%.2f\n",
           name,
           (unsigned long long)counter_value,
           (double)counter_value / (double)iterations);
    printf("%s: PMCR_PIPELINE_FREEZE_BY_DCACHE_MISS_MODE per us=%.2f\n",
           name,
           (elapsed_us > 0.0) ? ((double)counter_value / elapsed_us) : 0.0);
}

void test_bench_fullscreen_triangle_scanline_textured(void) {
    TEST_ASSERT_TRUE(init_test_render_buffers(64));
    reset_textures();

    texture_t texture = make_test_texture_4x4();
    int id = install_test_texture(texture);
    TEST_ASSERT_TRUE(id >= 0);

    triangle_t tri = make_fullscreen_triangle_1(id);
    const int iterations = 1000;

    perf_cntr_stop(PRFC1);
    perf_cntr_clear(PRFC1);
    perf_cntr_start(PRFC1, PMCR_PIPELINE_FREEZE_BY_DCACHE_MISS_MODE, PMCR_COUNT_CPU_CYCLES);

    uint64_t start = bench_now_ns();
    for (int i = 0; i < iterations; i++) {
        draw_textured_triangle_scanline(&tri, &texture);
    }
    uint64_t end = bench_now_ns();

    perf_cntr_stop(PRFC1);
    uint64_t freeze = perf_cntr_count(PRFC1);

    print_counter_result("triangle_scanline_textured", end - start, freeze, iterations);
    TEST_ASSERT_TRUE((end - start) > 0);
}

void test_bench_fullscreen_triangle_scanline_flat(void) {
    TEST_ASSERT_TRUE(init_test_render_buffers(64));

    triangle_t tri = make_fullscreen_triangle_1(0);
    const int iterations = 1000;

    perf_cntr_stop(PRFC1);
    perf_cntr_clear(PRFC1);
    perf_cntr_start(PRFC1, PMCR_PIPELINE_FREEZE_BY_DCACHE_MISS_MODE, PMCR_COUNT_CPU_CYCLES);

    uint64_t start = bench_now_ns();
    for (int i = 0; i < iterations; i++) {
        draw_filled_triangle(&tri, 0xF800);
    }
    uint64_t end = bench_now_ns();

    perf_cntr_stop(PRFC1);
    uint64_t freeze = perf_cntr_count(PRFC1);

    print_counter_result("triangle_scanline_flat", end - start, freeze, iterations);
    TEST_ASSERT_TRUE((end - start) > 0);
}

void test_bench_fullscreen_quad_scanline_textured(void) {
    TEST_ASSERT_TRUE(init_test_render_buffers(64));
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

    print_bench_result("fullscreen_quad_scanline_textured", end - start, iterations, 0);
    TEST_ASSERT_TRUE((end - start) > 0);
}

void test_bench_fullscreen_quad_scanline_flat(void) {
    TEST_ASSERT_TRUE(init_test_render_buffers(64));

    triangle_t tri1 = make_fullscreen_triangle_1(0);
    triangle_t tri2 = make_fullscreen_triangle_2(0);
    const int iterations = 500;

    uint64_t start = bench_now_ns();
    for (int i = 0; i < iterations; i++) {
        draw_filled_triangle(&tri1, 0xF800);
        draw_filled_triangle(&tri2, 0xF800);
    }
    uint64_t end = bench_now_ns();

    print_bench_result("fullscreen_quad_scanline_flat", end - start, iterations, 0);
    TEST_ASSERT_TRUE((end - start) > 0);
}

/* Backward-compatible wrappers */
void test_bench_fullscreen_triangle_scanline(void) {
    test_bench_fullscreen_triangle_scanline_textured();
}

void test_bench_fullscreen_quad_scanline(void) {
    test_bench_fullscreen_quad_scanline_textured();
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

void test_bench_scanline_640_float_mask_unrolled4(void) {
    const int iterations = 10000;
    const int count = 640;
    const int shift = 8;
    const int w_mask = 255;
    const int h_mask = 255;

    static uint16_t tex_data[256 * 256];
    static uint16_t dst[640];

    float u0 = 0.0f;
    float v0 = 128.0f;
    float du_dx = 0.10f;
    float dv_dx = 0.03f;

    uint64_t start = bench_now_ns();
    for (int it = 0; it < iterations; it++) {
        float u = u0;
        float v = v0;

        int i = 0;
        for (; i <= count - 4; i += 4) {
            int tx0 = ((int)u) & w_mask;
            int ty0 = ((int)v) & h_mask;
            u += du_dx; v -= dv_dx;

            int tx1 = ((int)u) & w_mask;
            int ty1 = ((int)v) & h_mask;
            u += du_dx; v -= dv_dx;

            int tx2 = ((int)u) & w_mask;
            int ty2 = ((int)v) & h_mask;
            u += du_dx; v -= dv_dx;

            int tx3 = ((int)u) & w_mask;
            int ty3 = ((int)v) & h_mask;
            u += du_dx; v -= dv_dx;

            dst[i + 0] = tex_data[(ty0 << shift) + tx0];
            dst[i + 1] = tex_data[(ty1 << shift) + tx1];
            dst[i + 2] = tex_data[(ty2 << shift) + tx2];
            dst[i + 3] = tex_data[(ty3 << shift) + tx3];
        }

        for (; i < count; i++) {
            int tx = ((int)u) & w_mask;
            int ty = ((int)v) & h_mask;
            dst[i] = tex_data[(ty << shift) + tx];
            u += du_dx;
            v -= dv_dx;
        }
    }
    uint64_t end = bench_now_ns();

    g_draw_sink ^= sample_u16_buffer(dst, count);
    print_bench_result("scanline_640_float_mask_u4", end - start, iterations, 0);

    TEST_ASSERT_TRUE((end - start) > 0);
}

void test_bench_scanline_640_fixed_mask_unrolled4(void) {
    const int iterations = 10000;
    const int count = 640;
    const int shift = 8;
    const int w_mask = 255;
    const int h_mask = 255;

    static uint16_t tex_data[256 * 256];
    static uint16_t dst[640];

    const float u0f = 0.0f;
    const float v0f = 128.0f;
    const float du_dxf = 0.10f;
    const float dv_dxf = 0.03f;

    const int32_t u0    = (int32_t)(u0f * 65536.0f);
    const int32_t v0    = (int32_t)(v0f * 65536.0f);
    const int32_t du_dx = (int32_t)(du_dxf * 65536.0f);
    const int32_t dv_dx = (int32_t)(dv_dxf * 65536.0f);

    uint64_t start = bench_now_ns();
    for (int it = 0; it < iterations; it++) {
        int32_t u = u0;
        int32_t v = v0;

        int i = 0;
        for (; i <= count - 4; i += 4) {
            int tx0 = (u >> 16) & w_mask;
            int ty0 = (v >> 16) & h_mask;
            u += du_dx; v -= dv_dx;

            int tx1 = (u >> 16) & w_mask;
            int ty1 = (v >> 16) & h_mask;
            u += du_dx; v -= dv_dx;

            int tx2 = (u >> 16) & w_mask;
            int ty2 = (v >> 16) & h_mask;
            u += du_dx; v -= dv_dx;

            int tx3 = (u >> 16) & w_mask;
            int ty3 = (v >> 16) & h_mask;
            u += du_dx; v -= dv_dx;

            dst[i + 0] = tex_data[(ty0 << shift) + tx0];
            dst[i + 1] = tex_data[(ty1 << shift) + tx1];
            dst[i + 2] = tex_data[(ty2 << shift) + tx2];
            dst[i + 3] = tex_data[(ty3 << shift) + tx3];
        }

        for (; i < count; i++) {
            int tx = (u >> 16) & w_mask;
            int ty = (v >> 16) & h_mask;
            dst[i] = tex_data[(ty << shift) + tx];
            u += du_dx;
            v -= dv_dx;
        }
    }
    uint64_t end = bench_now_ns();

    g_draw_sink ^= sample_u16_buffer(dst, count);
    print_bench_result("scanline_640_fixed_mask_u4", end - start, iterations, 0);

    TEST_ASSERT_TRUE((end - start) > 0);
}

void test_bench_scanline_fill_only(void) {
    const int iterations = 10000;
    const int count = 640;
    const int shift = 8;
    const int w_mask = 255;
    const int h_mask = 255;

    static uint16_t tex_data[256 * 256];
    static uint16_t dst[640];

    float u0 = 0.0f;
    float v0 = 128.0f;
    float du_dx = 0.10f;
    float dv_dx = 0.03f;

    uint64_t start = bench_now_ns();
    for (int it = 0; it < iterations; it++) {
        float u = u0;
        float v = v0;

        for (int i = 0; i < count; i++) {
            int tx = ((int)u) & w_mask;
            int ty = ((int)v) & h_mask;
            dst[i] = tex_data[(ty << shift) + tx];
            u += du_dx;
            v -= dv_dx;
        }
    }
    uint64_t end = bench_now_ns();

    g_draw_sink ^= sample_u16_buffer(dst, count);
    print_bench_result("scanline_fill_only", end - start, iterations, 0);
    TEST_ASSERT_TRUE((end - start) > 0);
}

void test_bench_scanline_setup_only(void) {
    const int iterations = 100000;
    volatile float sink = 0.0f;

    float xL = 10.3f;
    float xR = 500.8f;
    float uL = 0.1f;
    float uR = 0.9f;
    float vL = 0.2f;
    float vR = 0.7f;
    const float u_scale = 255.0f;
    const float v_scale = 255.0f;

    uint64_t start = bench_now_ns();
    for (int i = 0; i < iterations; i++) {
        float span_width = xR - xL;
        float inv_span = shz_invf(span_width);
        float du_dx = (uR - uL) * inv_span * u_scale;
        float dv_dx = (vR - vL) * inv_span * v_scale;

        int x_start = (int)shz_floorf(xL);
        int x_end   = (int)shz_ceilf(xR);

        float dx = (float)x_start - xL;
        float u = (uL * u_scale) + dx * du_dx;
        float v = 255.0f - ((vL * v_scale) + dx * dv_dx);

        sink += du_dx + dv_dx + u + v + (float)x_end;

        xL += 0.001f;
        xR += 0.001f;
    }
    uint64_t end = bench_now_ns();

    g_draw_sink ^= (uint32_t)sink;
    print_bench_result("scanline_setup_only", end - start, iterations, 0);
    TEST_ASSERT_TRUE((end - start) > 0);
}
