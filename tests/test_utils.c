#include "test_utils.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <dc/minifont.h>
#include <dc/perfctr.h>

#define BENCH_LOG_CAPACITY 4096

static char g_bench_log[BENCH_LOG_CAPACITY];
static int g_bench_log_len = 0;

uint64_t bench_now_ns(void) {
    return perf_cntr_timer_ns();
}

void bench_log_reset(void) {
    g_bench_log[0] = '\0';
    g_bench_log_len = 0;
}

void bench_log_append(const char *fmt, ...) {
    if (g_bench_log_len >= BENCH_LOG_CAPACITY - 1) {
        return;
    }

    va_list args;
    va_start(args, fmt);

    int written = vsnprintf(
        g_bench_log + g_bench_log_len,
        BENCH_LOG_CAPACITY - g_bench_log_len,
        fmt,
        args
    );

    va_end(args);

    if (written <= 0) {
        return;
    }

    g_bench_log_len += written;
    if (g_bench_log_len >= BENCH_LOG_CAPACITY) {
        g_bench_log_len = BENCH_LOG_CAPACITY - 1;
        g_bench_log[g_bench_log_len] = '\0';
    }
}

const char *bench_log_get(void) {
    return g_bench_log;
}

int test_offset(int x, int y) {
    return y * TEST_SCREEN_WIDTH + x;
}

void fill_u16(uint16_t *dst, int count, uint16_t color) {
    for (int i = 0; i < count; i++) {
        dst[i] = color;
    }
}

void draw_multiline_text(uint16_t *buf, int width, int x, int y, const char *text) {
    char line[128];
    int line_len = 0;
    int cy = y;

    for (const char *p = text; ; p++) {
        if (*p == '\n' || *p == '\0') {
            line[line_len] = '\0';
            minifont_draw_str(buf + (cy * width + x), width, line);
            cy += 12;
            line_len = 0;

            if (*p == '\0') {
                break;
            }
        } else if (line_len < (int)sizeof(line) - 1) {
            line[line_len++] = *p;
        }
    }
}

void print_bench_result(const char *name, uint64_t total_ns, int iterations, size_t bytes) {
    double per_iter_ns = (double)total_ns / (double)iterations;
    double per_iter_ms = per_iter_ns / 1000000.0;

    if (bytes == 0) {
        bench_log_append("%s: %.2f ns  %.6f ms\n", name, per_iter_ns, per_iter_ms);
        printf("%s: %.2f ns  %.6f ms\n", name, per_iter_ns, per_iter_ms);
        return;
    }

    double mb_per_iter = (double)bytes / (1024.0 * 1024.0);
    double mbps = 0.0;

    if (per_iter_ms > 0.0) {
        mbps = mb_per_iter / (per_iter_ms / 1000.0);
    }

    bench_log_append("%s: %.3f ms  %.2f MB/s\n", name, per_iter_ms, mbps);
    printf("%s: %.3f ms  %.2f MB/s\n", name, per_iter_ms, mbps);
}
