#ifndef DEBUG_H
#define DEBUG_H

#include <kos.h>
#include <stdio.h>

#ifdef DEBUG_ENABLED
    void debug_profiler_start();
    void debug_profiler_stop();
    void debug_profiler_print(FILE *out);
    void debug_profiler_reset();
#else
    #define debug_profiler_start()
    #define debug_profiler_stop()
    #define debug_profiler_print(out)
    #define debug_profiler_reset()
#endif

#endif // DEBUG_H
