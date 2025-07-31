#ifndef utils_h
#define utils_h

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <png/png.h>

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

extern uint16_t pixel_offsets[640 * 480];

typedef struct {
    void* data; // Pointer to the dynamic array data
    size_t element_size; // Size of each element in the array
    size_t capacity; // Total capacity of the array
    size_t size; // Current number of elements in the array
} dynamic_array_t;


#define da_append(xs, x) do { \
    if ((xs)->size >= (xs)->capacity) { \
        (xs)->capacity = (xs)->capacity ? (xs)->capacity * 2 : 256; \
        (xs)->data = realloc((xs)->data, (xs)->capacity * (xs)->element_size); \
    } \
    memcpy((char*)(xs)->data + ((xs)->size * (xs)->element_size), &(x), (xs)->element_size); \
    (xs)->size++; \
} while (0)


void debug_start(void);
void debug_end(void);









#endif