#include "./testing.h/testing.h"
#include "./test_ranges.h"
#include "./test_bench.h"
#include "./test_mem_profiler.h"
#include "./test_bench_output.h"

SUB_TEST(all_tests, {&test_ranges, "Test Ranges"},
{&test_bench, "Test benchmarking"},
{&test_memory_profiler, "Test memory profiler"},
{&test_bench_output, "Test benchmarking output"})

int main()
{
    lprintf(LOG_INFO, "Running " PROJECT_NAME " tests.\n");


    unit_test tests[] = {
        {&all_tests, "All Tests"}
    };

    int res = run_tests(tests, sizeof(tests) / sizeof(*tests), "All Tests");
    return res == 0 ? 0 : 1;
}
