#include <kos.h>
#include <stdlib.h>
#include <stdio.h>
#include "../third_party/unity/unity.h"

#include "../include/display.h"
#include "../include/renderer.h"
#include "test_utils.h"

extern void test_bench_z_buffer_clear(void);
extern void test_bench_color_buffer_clear(void);
extern void test_bench_background_copy(void);
extern void test_bench_vram_copy(void);
extern void test_bench_fullscreen_triangle_scanline(void);
extern void test_bench_fullscreen_quad_scanline(void);
extern void test_bench_clamp_only(void);
extern void test_bench_fetch_only(void);
extern void test_bench_clamp_and_fetch(void);
extern void test_bench_fetch_no_clamp(void);
extern void test_bench_scanline_640_pixels_clamp(void);
extern void test_bench_scanline_640_pixels_no_clamp(void);

void setUp(void) {
}

void tearDown(void) {
}

void test_basic_sanity(void) {
    TEST_ASSERT_EQUAL_INT(4, 2 + 2);
}

int main(int argc, char **argv) {
    vid_set_mode(DM_640x480, PM_RGB565);

    dbgio_dev_select("console");
    dbgio_enable();

    uint16_t *test_buffer = (uint16_t *)aligned_alloc(32, TEST_SCREEN_BUF_SIZE);
    if (!test_buffer) {
        printf("failed to allocate test_buffer\n");
        return 1;
    }

    fill_u16(test_buffer, TEST_SCREEN_WIDTH * TEST_SCREEN_HEIGHT, 0xF800);
    bench_log_reset();

    UNITY_BEGIN();
    RUN_TEST(test_basic_sanity);
    RUN_TEST(test_bench_z_buffer_clear);
    RUN_TEST(test_bench_color_buffer_clear);
    RUN_TEST(test_bench_background_copy);
    RUN_TEST(test_bench_clamp_only);
    RUN_TEST(test_bench_fetch_only);
    RUN_TEST(test_bench_clamp_and_fetch);
    RUN_TEST(test_bench_fetch_no_clamp);
    RUN_TEST(test_bench_scanline_640_pixels_clamp);
    RUN_TEST(test_bench_scanline_640_pixels_no_clamp);
    RUN_TEST(test_bench_fullscreen_triangle_scanline);
    RUN_TEST(test_bench_fullscreen_quad_scanline);
    printf("\n==== Benchmark Results ====\n%s\n", bench_log_get());
    /* RUN_TEST(test_bench_vram_copy); */
    int failures = UNITY_END();

    minifont_draw_str(
        test_buffer + test_offset(20, 20),
        TEST_SCREEN_WIDTH,
        "Unity Bench Results"
    );

    draw_multiline_text(
        test_buffer,
        TEST_SCREEN_WIDTH,
        20,
        40,
        bench_log_get()
    );

    char footer[64];
    snprintf(footer, sizeof(footer), "Failures: %d", failures);
    minifont_draw_str(
        test_buffer + test_offset(20, TEST_SCREEN_HEIGHT - 20),
        TEST_SCREEN_WIDTH,
        footer
    );

    sq_lock((void *)((uint8_t *)vram_s));
    shz_sq_memcpy32(
        (void *)((uint8_t *)vram_s),
        (const void *)((uint8_t *)test_buffer),
        TEST_SCREEN_BUF_SIZE
    );
    sq_unlock();

    for (;;) {
    }

    return 0;
}