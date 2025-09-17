#include "debug.h"
#include <stdio.h>

static uint64_t start_time = 0;
static uint64_t accumulated_time = 0;
static int frame_limit = 0;
static int frame_count = 0;

void init_timer(int frames) {
    accumulated_time = 0;
    frame_limit = frames;
    frame_count = 0;
    
}

void start_timer() {
    start_time = perf_cntr_timer_ns();
}

void end_timer() {
    uint64_t end_time = perf_cntr_timer_ns();
    uint64_t elapsed = end_time - start_time;
    accumulated_time += elapsed;
    frame_count++;
    if(frame_count == frame_limit) {
        isRunning = false;
    }
    
}

void print_debug_info(const char* message) {
    uint64_t avg = accumulated_time / frame_count;
    printf("%s average over %d frames: %llu ns\n", message, frame_count, avg);  
}
