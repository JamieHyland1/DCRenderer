#include "unity.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "shz_mem.h"
#include <dc/perfctr.h>

#include "test_utils.h"

#ifndef BENCH_ITERS_SMALL
#define BENCH_ITERS_SMALL 64
#endif

#ifndef BENCH_ITERS_LARGE
#define BENCH_ITERS_LARGE 256
#endif

static volatile uint32_t g_bench_sink = 0;

static inline uint64_t bench_now_ns(void) {
    return perf_cntr_timer_ns();
}

static void warmup_memory(const void *src, void *dst, size_t size) {
    volatile const uint8_t *s = (const uint8_t *)src;
    volatile uint8_t *d = (uint8_t *)dst;

    for (size_t i = 0; i < size; i += 64) {
        d[i] = s ? s[i] : 0;
    }
}

static uint32_t sample_bytes_u8(const void *ptr, size_t size) {
    const uint8_t *p = (const uint8_t *)ptr;
    uint32_t sum = 0;

    for (size_t i = 0; i < size; i += 4096) {
        sum += p[i];
    }

    if (size > 0) {
        sum += p[size - 1];
    }

    return sum;
}

void test_bench_z_buffer_clear(void) {
    const int iterations = BENCH_ITERS_SMALL;
    const size_t zbytes =
        (size_t)TEST_SCREEN_WIDTH * (size_t)TEST_SCREEN_HEIGHT * sizeof(float);

    float *zb = (float *)aligned_alloc(32, zbytes);
    TEST_ASSERT_NOT_NULL(zb);

    memset(zb, 0x7F, zbytes);
    warmup_memory(zb, zb, zbytes);

    uint64_t start = bench_now_ns();
    for (int i = 0; i < iterations; i++) {
        memset(zb, 0, zbytes);
    }
    uint64_t end = bench_now_ns();

    g_bench_sink ^= sample_bytes_u8(zb, zbytes);

    print_bench_result("z_buffer_clear", end - start, iterations, zbytes);

    free(zb);
    TEST_ASSERT_TRUE((end - start) > 0);
}

void test_bench_color_buffer_clear(void) {
    const int iterations = BENCH_ITERS_LARGE;
    const size_t cbytes =
        (size_t)TEST_SCREEN_WIDTH * (size_t)TEST_SCREEN_HEIGHT * sizeof(uint16_t);

    uint16_t *buf = (uint16_t *)aligned_alloc(32, cbytes);
    TEST_ASSERT_NOT_NULL(buf);

    memset(buf, 0x7F, cbytes);
    warmup_memory(buf, buf, cbytes);

    uint64_t start = bench_now_ns();
    for (int i = 0; i < iterations; i++) {
        memset(buf, 0, cbytes);
    }
    uint64_t end = bench_now_ns();

    g_bench_sink ^= sample_bytes_u8(buf, cbytes);

    print_bench_result("color_buffer_clear", end - start, iterations, cbytes);

    free(buf);
    TEST_ASSERT_TRUE((end - start) > 0);
}

void test_bench_background_copy(void) {
    const int iterations = BENCH_ITERS_LARGE;
    const size_t cbytes =
        (size_t)TEST_SCREEN_WIDTH * (size_t)TEST_SCREEN_HEIGHT * sizeof(uint16_t);

    uint16_t *src = (uint16_t *)aligned_alloc(32, cbytes);
    uint16_t *dst = (uint16_t *)aligned_alloc(32, cbytes);
    TEST_ASSERT_NOT_NULL(src);
    TEST_ASSERT_NOT_NULL(dst);

    memset(src, 0xAA, cbytes);
    memset(dst, 0x11, cbytes);
    warmup_memory(src, dst, cbytes);

    uint64_t start = bench_now_ns();
    for (int i = 0; i < iterations; i++) {
        memcpy(dst, src, cbytes);
    }
    uint64_t end = bench_now_ns();

    g_bench_sink ^= sample_bytes_u8(dst, cbytes);

    print_bench_result("background_copy", end - start, iterations, cbytes);

    free(src);
    free(dst);
    TEST_ASSERT_TRUE((end - start) > 0);
}

void test_bench_background_copy_shz(void) {
    const int iterations = BENCH_ITERS_LARGE;
    const size_t cbytes =
        (size_t)TEST_SCREEN_WIDTH * (size_t)TEST_SCREEN_HEIGHT * sizeof(uint16_t);

    TEST_ASSERT_EQUAL_UINT32(0, cbytes % 32);

    uint16_t *src = aligned_alloc(32, cbytes);
    uint16_t *dst = aligned_alloc(32, cbytes);
    TEST_ASSERT_NOT_NULL(src);
    TEST_ASSERT_NOT_NULL(dst);

    memset(src, 0xAA, cbytes);
    memset(dst, 0x11, cbytes);
    warmup_memory(src, dst, cbytes);

    uint64_t start = bench_now_ns();
    for (int i = 0; i < iterations; i++) {
        shz_memcpy32(dst, src, cbytes);
    }
    uint64_t end = bench_now_ns();

    g_bench_sink ^= sample_bytes_u8(dst, cbytes);
    print_bench_result("background_copy_shz_memcpy32", end - start, iterations, cbytes);

    free(src);
    free(dst);
    TEST_ASSERT_TRUE((end - start) > 0);
}

void test_bench_background_copy_shz_sq(void) {
    const int iterations = BENCH_ITERS_LARGE;
    const size_t cbytes =
        (size_t)TEST_SCREEN_WIDTH * (size_t)TEST_SCREEN_HEIGHT * sizeof(uint16_t);

    TEST_ASSERT_EQUAL_UINT32(0, cbytes % 32);

    uint16_t *src = aligned_alloc(32, cbytes);
    uint16_t *dst = aligned_alloc(32, cbytes);
    TEST_ASSERT_NOT_NULL(src);
    TEST_ASSERT_NOT_NULL(dst);

    memset(src, 0xAA, cbytes);
    memset(dst, 0x11, cbytes);
    warmup_memory(src, dst, cbytes);

    uint64_t start = bench_now_ns();
    for (int i = 0; i < iterations; i++) {
        shz_sq_memcpy32(dst, src, cbytes);
    }
    uint64_t end = bench_now_ns();

    g_bench_sink ^= sample_bytes_u8(dst, cbytes);
    print_bench_result("background_copy_shz_sq_memcpy32", end - start, iterations, cbytes);

    free(src);
    free(dst);
    TEST_ASSERT_TRUE((end - start) > 0);
}

void test_bench_z_buffer_clear_template_copy(void) {
    const int iterations = BENCH_ITERS_LARGE;
    const size_t count = (size_t)TEST_SCREEN_WIDTH * (size_t)TEST_SCREEN_HEIGHT;
    const size_t zbytes = count * sizeof(float);

    /* shz_memcpy32 requires dst 32-byte aligned and bytes multiple of 32 */
    TEST_ASSERT_EQUAL_UINT32(0, zbytes % 32);

    float *z_buffer = (float *)aligned_alloc(32, zbytes);
    float *z_template = (float *)aligned_alloc(32, zbytes);
    TEST_ASSERT_NOT_NULL(z_buffer);
    TEST_ASSERT_NOT_NULL(z_template);

    for (size_t i = 0; i < count; i++) {
        z_template[i] = 1.0f;
        z_buffer[i] = 0.0f;
    }

    warmup_memory(z_template, z_buffer, zbytes);

    uint64_t start = bench_now_ns();
    for (int i = 0; i < iterations; i++) {
        shz_memcpy32(z_buffer, z_template, zbytes);
    }
    uint64_t end = bench_now_ns();

    g_bench_sink ^= sample_bytes_u8((const uint8_t *)z_buffer, zbytes);

    print_bench_result("z_buffer_clear_template_copy", end - start, iterations, zbytes);

    free(z_buffer);
    free(z_template);
    TEST_ASSERT_TRUE((end - start) > 0);
}

void test_bench_z_buffer_clear_float_loop(void) {
    const int iterations = BENCH_ITERS_LARGE;
    const size_t count = (size_t)TEST_SCREEN_WIDTH * (size_t)TEST_SCREEN_HEIGHT;
    const size_t zbytes = count * sizeof(float);

    float *z_buffer = (float *)aligned_alloc(32, zbytes);
    TEST_ASSERT_NOT_NULL(z_buffer);

    for (size_t i = 0; i < count; i++) {
        z_buffer[i] = 0.0f;
    }

    warmup_memory(z_buffer, z_buffer, zbytes);

    uint64_t start = bench_now_ns();
    for (int it = 0; it < iterations; it++) {
        for (size_t i = 0; i < count; i++) {
            z_buffer[i] = 1.0f;
        }
    }
    uint64_t end = bench_now_ns();

    g_bench_sink ^= sample_bytes_u8((const uint8_t *)z_buffer, zbytes);

    print_bench_result("z_buffer_clear_float_loop", end - start, iterations, zbytes);

    free(z_buffer);
    TEST_ASSERT_TRUE((end - start) > 0);
}

void test_bench_vram_copy(void) {
    TEST_IGNORE_MESSAGE("Run VRAM copy benchmark separately.");
}
