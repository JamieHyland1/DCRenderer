#ifndef DEBUG_H
#define DEBUG_H
#include "display.h"
#include <stdint.h>
extern uint64_t avg;
extern uint64_t start_time;
extern uint64_t end_time;
// Initialize the timer to average over N frames
void init_timer(int frames);

// Mark the start of a timed section
void start_timer();

// Mark the end of a timed section and optionally print the average
void end_timer();

void print_debug_info(const char* message);

#endif // DEBUG_H
