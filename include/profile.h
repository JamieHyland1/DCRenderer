#ifndef PROFILE_H
#define PROFILE_H

#include <stdint.h>

typedef enum {
    PROF_RENDER = 0,
    PROF_PIPELINE,
    PROF_COPY,
    PROF_COUNT
} profile_section_t;

#ifdef DEBUG

#include <dc/perfctr.h>

typedef enum {
    PROF_EVENT_DCACHE_FREEZE = 0,
    PROF_EVENT_DCACHE_READ_MISS,
    PROF_EVENT_DCACHE_WRITE_MISS,
    PROF_EVENT_BRANCH_TAKEN,
    PROF_EVENT_FPU_ISSUED,
    PROF_EVENT_INSTRUCTION_ISSUED,
    PROF_EVENT_COUNT
} profile_event_t;

void profile_reset(void);
void profile_begin(profile_section_t section);
void profile_end(profile_section_t section);
void profile_print(void);

void profile_perf_start(profile_event_t event);
uint64_t profile_perf_stop(void);

#define PROFILE_RESET()              profile_reset()
#define PROFILE_BEGIN(section)       profile_begin(section)
#define PROFILE_END(section)         profile_end(section)
#define PROFILE_PRINT()              profile_print()
#define PROFILE_PERF_START(event)    profile_perf_start(event)
#define PROFILE_PERF_STOP()          profile_perf_stop()

#else

#define PROFILE_RESET()              do { } while (0)
#define PROFILE_BEGIN(section)       do { (void)(section); } while (0)
#define PROFILE_END(section)         do { (void)(section); } while (0)
#define PROFILE_PRINT()              do { } while (0)
#define PROFILE_PERF_START(event)    do { (void)(event); } while (0)
#define PROFILE_PERF_STOP()          ((uint64_t)0)

#endif

#endif