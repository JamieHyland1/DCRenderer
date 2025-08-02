#ifndef utils_h
#define utils_h
#include <png/png.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <tgmath.h>
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

void debug_start(void);
void debug_end(void);









#endif