#ifndef utils_h
#define utils_h
#include <png/png.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include <kos.h>
#include <dc/video.h>
#include <dc/fmath.h>
#include <dc/maple.h>
#include <dc/biosfont.h>
#include <dc/maple/controller.h>
#include <utime.h>  // for timer_us_gettime64
#include <kos/thread.h>
#include <dc/minifont.h>
#include <dc/perf_monitor.h>

#define CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))

#define SWAP(v1, v2, type) { \
type temp = v2; \
v2 = v1; \
v1 = temp; \
}

#define SORT_AND_FIX_TRIANGLE(points, texcoords) do {                           \
    /* Sort vertices by Y */                                                    \
    if ((points)[0].y > (points)[1].y) {                                        \
        SWAP((points)[0], (points)[1], shz_vec4_t);                             \
        SWAP((texcoords)[0], (texcoords)[1], tex2_t);                           \
    }                                                                           \
    if ((points)[1].y > (points)[2].y) {                                        \
        SWAP((points)[1], (points)[2], shz_vec4_t);                             \
        SWAP((texcoords)[1], (texcoords)[2], tex2_t);                           \
    }                                                                           \
    if ((points)[0].y > (points)[1].y) {                                        \
        SWAP((points)[0], (points)[1], shz_vec4_t);                             \
        SWAP((texcoords)[0], (texcoords)[1], tex2_t);                           \
    }                                                                           \
                                                                                \
    /* Winding correction */                                                    \
    float area = ((points)[1].x - (points)[0].x) * ((points)[2].y - (points)[0].y) - \
                 ((points)[2].x - (points)[0].x) * ((points)[1].y - (points)[0].y); \
    if (area < 0.0f) {                                                          \
        SWAP((points)[1], (points)[2], shz_vec4_t);                             \
        SWAP((texcoords)[1], (texcoords)[2], tex2_t);                           \
    }                                                                           \
} while (0)


void debug_start(void);
void debug_end(void);









#endif
