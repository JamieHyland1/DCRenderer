#include "profile.h"

#ifdef DEBUG

#include <stdio.h>
#include <kos.h>
#include <dc/perfctr.h>

typedef struct {
    uint64_t total_us;
    uint64_t start_us;
    uint32_t calls;
} profile_section_data_t;

static profile_section_data_t g_sections[PROF_COUNT];

static const char *section_name(profile_section_t section) {
    switch (section) {
        case PROF_RENDER:   return "render";
        case PROF_PIPELINE: return "pipeline";
        case PROF_COPY:     return "copy";
        default:            return "unknown";
    }
}

static perf_cntr_event_t event_to_pmcr(profile_event_t event) {
    switch (event) {
        case PROF_EVENT_DCACHE_FREEZE:
            return PMCR_PIPELINE_FREEZE_BY_DCACHE_MISS_MODE;

        case PROF_EVENT_DCACHE_READ_MISS:
            return PMCR_OPERAND_CACHE_READ_MISS_MODE;

        case PROF_EVENT_DCACHE_WRITE_MISS:
            return PMCR_OPERAND_CACHE_WRITE_MISS_MODE;

        case PROF_EVENT_BRANCH_TAKEN:
            return PMCR_BRANCH_TAKEN_MODE;

        case PROF_EVENT_FPU_ISSUED:
            return PMCR_FPU_INSTRUCTION_ISSUED_MODE;

        case PROF_EVENT_INSTRUCTION_ISSUED:
            return PMCR_INSTRUCTION_ISSUED_MODE;

        default:
            return PMCR_INSTRUCTION_ISSUED_MODE;
    }
}

void profile_reset(void) {
    for (int i = 0; i < PROF_COUNT; i++) {
        g_sections[i].total_us = 0;
        g_sections[i].start_us = 0;
        g_sections[i].calls = 0;
    }
}

void profile_begin(profile_section_t section) {
    g_sections[section].start_us = timer_us_gettime64();
}

void profile_end(profile_section_t section) {
    uint64_t end = timer_us_gettime64();
    g_sections[section].total_us += end - g_sections[section].start_us;
    g_sections[section].calls++;
}

void profile_print(void) {
    for (int i = 0; i < PROF_COUNT; i++) {
        if (g_sections[i].calls == 0) continue;

        double avg_ms =
            (double)g_sections[i].total_us /
            (double)g_sections[i].calls /
            1000.0;

        printf("%s: avg %.3f ms over %lu calls\n",
               section_name((profile_section_t)i),
               avg_ms,
               (unsigned long)g_sections[i].calls);
    }
}

void profile_perf_start(profile_event_t event) {
    perf_cntr_clear(PRFC1);

    perf_cntr_start(
        PRFC1,
        event_to_pmcr(event),
        PMCR_COUNT_CPU_CYCLES
    );
}

uint64_t profile_perf_stop(void) {
    perf_cntr_stop(PRFC1);
    return perf_cntr_count(PRFC1);
}

#endif