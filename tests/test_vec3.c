#include "../third_party/unity/unity.h"
#include "../include/vector.h"
#include "shz_trig.h"

void test_vec3_dot_basis(void) {
    shz_vec3_t a = vec3_new(1.0f, 0.0f, 0.0f);
    shz_vec3_t b = vec3_new(0.0f, 1.0f, 0.0f);

    float d = shz_vec_dot(a, b);

    TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, d);
}
