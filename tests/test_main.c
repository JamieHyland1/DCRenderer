#include <kos.h>
#include <stdlib.h>
#include <stdio.h>
#include "../third_party/unity/unity.h"

#include "../include/display.h"
#include "../include/renderer.h"
#include "../include/texture.h"
#include "test_utils.h"
#include "test_drawing.h"

extern void test_bench_z_buffer_clear(void);
extern void test_bench_color_buffer_clear(void);
extern void test_bench_background_copy(void);
extern void test_bench_background_copy_shz(void);
extern void test_bench_background_copy_shz_sq(void);
extern void test_bench_z_buffer_clear_template_copy(void);
extern void test_bench_z_buffer_clear_float_loop(void);
extern void test_bench_vram_copy(void);
extern void test_bench_clamp_only(void);
extern void test_bench_fetch_only(void);
extern void test_bench_clamp_and_fetch(void);
extern void test_bench_fetch_no_clamp(void);
extern void test_bench_scanline_640_pixels_clamp(void);
extern void test_bench_scanline_640_pixels_no_clamp(void);
extern void test_bench_scanline_fill_only(void);
extern void test_bench_scanline_setup_only(void);

void setUp(void) {
}

void tearDown(void) {
}

void test_basic_sanity(void) {
    TEST_ASSERT_EQUAL_INT(4, 2 + 2);
}

static void blit_png_to_buffer_scaled(uint16_t *dst, const kos_img_t *img) {
    fill_u16(dst, TEST_SCREEN_WIDTH * TEST_SCREEN_HEIGHT, 0x0000);

    if (img->fmt != KOS_IMG_FMT_RGB565) {
        printf("PNG format is not RGB565, fmt=%d\n", img->fmt);
        return;
    }

    const uint16_t *src = (const uint16_t *)img->data;

    for (int y = 0; y < TEST_SCREEN_HEIGHT; y++) {
        int src_y = (y * img->h) / TEST_SCREEN_HEIGHT;

        for (int x = 0; x < TEST_SCREEN_WIDTH; x++) {
            int src_x = (x * img->w) / TEST_SCREEN_WIDTH;
            dst[y * TEST_SCREEN_WIDTH + x] = src[src_y * img->w + src_x];
        }
    }
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

    const char jamie_space_sjis[] = {
        (char)0x83, (char)0x57,
        (char)0x83, (char)0x46,
        (char)0x83, (char)0x43,
        (char)0x83, (char)0x7E,
        (char)0x81, (char)0x5B,
        (char)0x82, (char)0xCC,
        (char)0x89, (char)0xBC,
        (char)0x91, (char)0x7A,
        (char)0x8B, (char)0xF3,
        (char)0x8A, (char)0xD4,
        0
    };

    kos_img_t bg;
    memset(&bg, 0, sizeof(bg));

    if (png_to_img("/rd/yokohama.png", 0, &bg) == 0) {
        printf("Loaded yokohama.png (%d x %d)\n", bg.w, bg.h);
        blit_png_to_buffer_scaled(test_buffer, &bg);
    } else {
        printf("Failed to load /rd/yokohama.png\n");
    }

    UNITY_BEGIN();
    RUN_TEST(test_basic_sanity);
    RUN_TEST(test_bench_scanline_640_fixed_mask_unrolled4);
    RUN_TEST(test_bench_scanline_fill_only);
    RUN_TEST(test_bench_scanline_setup_only);
    RUN_TEST(test_bench_fullscreen_triangle_scanline_textured);
    RUN_TEST(test_bench_fullscreen_triangle_scanline_flat);
    RUN_TEST(test_bench_fullscreen_quad_scanline_textured);
    RUN_TEST(test_bench_fullscreen_quad_scanline_flat);

    printf("\n==== Benchmark Results ====\n%s\n", bench_log_get());
    int failures = UNITY_END();

    pvr_init_defaults();

    bfont_set_encoding(BFONT_CODE_SJIS);
    bfont_set_foreground_color(0xFFFF);
    bfont_set_background_color(0x0000);

    sq_lock((void *)((uint8_t *)vram_s));
    shz_sq_memcpy32(
        (void *)((uint8_t *)vram_s),
        (const void *)((uint8_t *)test_buffer),
        TEST_SCREEN_BUF_SIZE
    );
    sq_unlock();

    bfont_draw_str_vram_fmt(190, 60, 0, "%s", jamie_space_sjis);
    bfont_draw_str_vram_fmt(10, 160, 20, "%s", bench_log_get());

    while (1) {
        maple_device_t *cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
        if (cont) {
            cont_state_t *state = (cont_state_t *)maple_dev_status(cont);
            if (state && (state->buttons & CONT_START)) {
                break;
            }
        }

        thd_sleep(16);
    }

    free(test_buffer);
    return failures;
}
