#include "unity.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <dc/perfctr.h>

#include "test_utils.h"
#include "../include/core.h"
#include "test_fixtures.h"
#include "test_drawing.h"

void test_pipeline(void) {
    const int iterations = 100;

    for (int i = 0; i < iterations; i++) {
        create_object("cube", i);
    }

    printf("number of objects in scene: %d\n", get_num_objects());

    perf_cntr_stop(PRFC1);
    perf_cntr_clear(PRFC1);
    perf_cntr_start(PRFC1,
                    PMCR_PIPELINE_FREEZE_BY_DCACHE_MISS_MODE,
                    PMCR_COUNT_CPU_CYCLES);

    uint64_t start = bench_now_ns();

    for (int i = 0; i < iterations; i++) {
        object_t obj;
        int index = i;

        if (get_object(index, &obj)) {
            float x = i * 5.5f;

            mesh_t *mesh = get_mesh(obj.id);
            if (mesh) {
                mesh->translation = vec3_new(mesh->translation.x, mesh->translation.y, mesh->translation.z);
            }

            process_graphics_pipeline(&obj);
        }
    }

    uint64_t end = bench_now_ns();

    perf_cntr_stop(PRFC1);
    uint64_t freeze = perf_cntr_count(PRFC1);

    print_counter_result("test_pipeline", end - start, freeze, iterations);
    TEST_ASSERT_TRUE((end - start) > 0);
}