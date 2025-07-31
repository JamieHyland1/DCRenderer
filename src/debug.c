#include "../include/debug.h"
#include <dc/perf_monitor.h>
#ifdef DEBUG_ENABLED
uint64_t cycles = 0;
void debug_profiler_start() {
    perf_monitor();
}

void debug_profiler_stop() {
    perf_monitor_exit();
}

void debug_profiler_print(FILE *out) {
    perf_monitor_print(out);
}

#endif
