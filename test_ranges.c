#include "./testing.h/testing.h"
#include "./ranges.h"
#include <string.h>
#include <math.h>

#define STEP 10
#define RANGE_LEN 10
#define END_START_OFFSET RANGE_LEN * STEP
#define END_FROM_START(i) i + END_START_OFFSET

#define DIFF(a, b) fabs((a) - (b))
#define THRESHHOLD 0.
#define D_EQUALS(a, b) islessequal(DIFF(a, b), THRESHHOLD)

int test_range_itt()
{
    // Test the default
    range_t range = RANGE_T_DEFAULT;
    ASSERT(range.current == 0);
    ASSERT(range.start == 0);
    ASSERT(range.end == 0);
    ASSERT(range.step == 0);

    // Test range start and, next
    for (int i = 0; i < 100; i++) {
        range_t range;
        range.start = i;
        range.end = END_FROM_START(i);
        range.step = STEP;

        range_start(&range);
        ASSERT(range.start == i);
        ASSERT(range.end == END_FROM_START(i));
        ASSERT(range.step == STEP);
        ASSERT(range.current == range.start);

        double output;
        for (int j = 0; j < RANGE_LEN; j++) {
            ASSERT(range_next(&range, &output) == RANGE_GENERATING);
            ASSERT(range.current == output);
            ASSERT(D_EQUALS(output, range.start + STEP * (j + 1)));

            ASSERT(range.start == i);
            ASSERT(range.end == END_FROM_START(i));
            ASSERT(range.step == STEP);
        }

        output = -1;
        ASSERT(range_next(&range, &output) == RANGE_STOPPED);
        ASSERT(output == -1);
    }

    return 1;
}

#define START 10
#define END 50
#define ITEMS_PER_RANGE (1 + (abs(START - END) / STEP))
#define ITEMS_PER_DIMENSION(d) pow(ITEMS_PER_RANGE, d)
#define DIMENSIONS 10
static int test_mdd_range_itt()
{
    // Init ranges
    range_t *ranges = malloc(sizeof(*ranges) * DIMENSIONS);
    ASSERT(ranges != NULL);
    for (size_t d = 0; d < DIMENSIONS; d++) {
        ranges[d].start = START;
        ranges[d].end = END;
        ranges[d].step = STEP;
    }

    // test init and, next
    for (size_t d/*imensions*/ = 1; d < DIMENSIONS; d++) {
        multi_dimensional_range_t range;
        memset(&range, 0, sizeof(range));

        ASSERT(init_multi_dimensional_range_arr(&range, d, ranges));
        ASSERT(range.dimensions == d);
        ASSERT(range.ranges != NULL);
        multi_dimensional_range_start(&range);

        ASSERT(range.dimensions == d);

        vector_t out;
        for (size_t i = 0; i < floor(ITEMS_PER_DIMENSION(d)); i++) {
            ASSERT(multi_dimensional_range_next(&range, &out) == RANGE_GENERATING);
            ASSERT(out.dimensions == range.dimensions);
            ASSERT(out.values != NULL);
            free_vector(&out);
        }
        ASSERT(multi_dimensional_range_next(&range, &out) == RANGE_STOPPED);

        free_multi_dimensional_range(&range);
    }

    free(ranges);
    return 1;
}

static int test_mdd_range_itt_2()
{
    multi_dimensional_range_t range;
    memset(&range, 0, sizeof(range));

    range_t range_1, range_2, range_3;
    range_1.start = range_2.start = range_3.start = 1;
    range_1.end = range_2.end = range_3.end = 3;
    range_1.step = range_2.step = range_3.step = 1;

    init_multi_dimensional_range(&range, range_1, range_2, range_3);
    ASSERT(range.dimensions == 3);
    ASSERT(range.ranges != NULL);
    ASSERT(memcmp(&range.ranges[0], &range_1, sizeof(range_1)) == 0);
    ASSERT(memcmp(&range.ranges[1], &range_2, sizeof(range_2)) == 0);
    ASSERT(memcmp(&range.ranges[2], &range_3, sizeof(range_3)) == 0);

    multi_dimensional_range_start(&range);

    vector_t out;
    for (size_t i = 0; i < 3 * 3 * 3; i++) {
        ASSERT(multi_dimensional_range_next(&range, &out) == RANGE_GENERATING);
        ASSERT(out.dimensions== range.dimensions);
        ASSERT(out.values != NULL);
        free_vector(&out);
    }

    ASSERT(multi_dimensional_range_next(&range, &out) == RANGE_STOPPED);

    free_multi_dimensional_range(&range);

    return 1;
}

SUB_TEST(test_ranges, {&test_range_itt, "Test range itt"},
{&test_mdd_range_itt, "Test multi dimensional range itt"},
{&test_mdd_range_itt_2, "Test multi dimensional range itt with other init method"})
