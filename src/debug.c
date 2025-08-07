#include "../include/renderer.h"
#ifdef DEBUG_ENABLED

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
