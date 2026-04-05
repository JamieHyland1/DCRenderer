
#include <kos.h>
#include "../third_party/unity/unity.h"

void setUp(void) {}
void tearDown(void) {}

void test_basic_sanity(void) {
    TEST_ASSERT_EQUAL_INT(4, 2 + 2);
}

int main(int argc, char **argv) {
    dbgio_dev_select("console");
    dbgio_enable();

    UNITY_BEGIN();
    RUN_TEST(test_basic_sanity);
    int failures = UNITY_END();

    printf("\nFailures: %d\n", failures);

    for (;;) {
    }

    return failures;
}
