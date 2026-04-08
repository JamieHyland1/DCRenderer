#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <stdint.h>
#include <stddef.h>

#define TEST_SCREEN_WIDTH  640
#define TEST_SCREEN_HEIGHT 480
#define TEST_SCREEN_BUF_SIZE (TEST_SCREEN_WIDTH * TEST_SCREEN_HEIGHT * sizeof(uint16_t))


inline uint64_t bench_now_ns(void);


/* Simple on-screen benchmark log */
void bench_log_reset(void);
void bench_log_append(const char *fmt, ...);
const char *bench_log_get(void);

/* Screen / text helpers */
int test_offset(int x, int y);
void fill_u16(uint16_t *dst, int count, uint16_t color);
void draw_multiline_text(uint16_t *buf, int width, int x, int y, const char *text);

/* Benchmark formatting helper */
void print_bench_result(const char *name, uint64_t total_ns, int iterations, size_t bytes);

#endif
