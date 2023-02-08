#include "./test_bench.h"
#include "./testing.h/testing.h"
#include "./bench.h"
#include "./ranges.h"
#include <string.h>
#include <math.h>

#define RUNS_TO_AVERAGE 1000

// Random longish sample function for benchmarking
static int example_func_np()
{
    int x = 0;
    for (int i = 0; i < 1000000; i++) {
        x += x * sin(x);
    }

    return 1;
}

static benchmark_conf_t get_conf_np()
{
    benchmark_func_type_t type = FUNC_NO_PARAM;
    int (*func_np)() = &example_func_np;

    benchmark_conf_t conf;
    memset(&conf, 0, sizeof(conf));
    conf.runs_to_average = RUNS_TO_AVERAGE;
    conf.function_type = type;
    conf.np_func = func_np;

    return conf;
}

static int example_func_p(vector_t vector)
{
    int ret = 2;
    for (size_t i = 0; i < 500; i++) {
        for (size_t i = 0; i < vector.dimensions; i++) {
            ret += pow(2, 10 * vector.values[i]);
        }
    }

    return 1;
}

#define LEN_EXPECTED_P (3 * 3 * 3)

static benchmark_conf_t get_conf_p()
{
    benchmark_func_type_t type = FUNC_PARAM;
    int (*func_p)(vector_t v) = &example_func_p;

    // Copied this from ranges.h
    multi_dimensional_range_t range;
    memset(&range, 0, sizeof(range));

    range_t range_1, range_2, range_3;
    range_1.start = range_2.start = range_3.start = 1;
    range_1.end = range_2.end = range_3.end = 3;
    range_1.step = range_2.step = range_3.step = 1;

    init_multi_dimensional_range(&range, range_1, range_2, range_3);

    benchmark_param_conf_t param_conf;
    param_conf.params_generator = range;

    benchmark_conf_t conf;
    memset(&conf, 0, sizeof(conf));
    conf.runs_to_average = RUNS_TO_AVERAGE;
    conf.function_type = type;
    conf.p_func = func_p;
    conf.param_conf = param_conf;

    return conf;
}

/// Profiles CPU time over 100 runs, this is the bare minimum settings for benchmarking
static int test_cpu_time_bench_np()
{
    benchmark_conf_t conf = get_conf_np();

    // Run the bench
    benchmark_profile_t output_profile;
    ASSERT(benchmark_program(&conf, &output_profile));
    ASSERT(output_profile.entries != NULL);
    lprintf(LOG_INFO, "example_func_np time is %lu us\n", output_profile.entries->cpu_time_us);

    ASSERT(output_profile.len == 1);
    ASSERT(output_profile.entries->cpu_time_us > 0);

    free_benchmark_profile(&output_profile);
    return 1;
}

static int test_cpu_time_bench_p()
{
    benchmark_conf_t conf = get_conf_p();

    // Run the bench
    benchmark_profile_t output_profile;
    ASSERT(benchmark_program(&conf, &output_profile));
    ASSERT(output_profile.entries != NULL);
    ASSERT(output_profile.len == LEN_EXPECTED_P);

    lprintf(LOG_INFO, "example_func_p time is %lu us\n", output_profile.entries[LEN_EXPECTED_P - 1].cpu_time_us);

    long t = 0;
    for (size_t i = 0; i < output_profile.len; i++) {
        t += output_profile.entries[i].cpu_time_us;
    }
    ASSERT(t > 0);

    free_multi_dimensional_range(&conf.param_conf.params_generator);
    free_benchmark_profile(&output_profile);
    return 1;
}

static int test_output_conf()
{
    benchmark_output_conf_t output_conf;
    ASSERT(init_benchmark_output_conf(&output_conf, OUTPUT_JSON, "testing_cpu_time_bench_np"));
    free_benchmark_output_conf(&output_conf);
    return 1;
}

static int test_mem_profiler_bench_p_0()
{
    benchmark_conf_t conf = get_conf_p();
    conf.mem_conf.enabled = 1;
    conf.mem_conf.poll_time = 1;

    // Run the bench
    benchmark_profile_t output_profile;
    ASSERT(benchmark_program(&conf, &output_profile));
    ASSERT(output_profile.entries != NULL);
    ASSERT(output_profile.len == LEN_EXPECTED_P);

    lprintf(LOG_INFO, "example_func_p time is %lu us\n", output_profile.entries[LEN_EXPECTED_P - 1].cpu_time_us);

    long t = 0;
    for (size_t i = 0; i < output_profile.len; i++) {
        t += output_profile.entries[i].cpu_time_us;
        ASSERT(output_profile.entries[i].max_mem_usage == 0);
    }
    ASSERT(t > 0);

    free_multi_dimensional_range(&conf.param_conf.params_generator);
    free_benchmark_profile(&output_profile);
    return 1;
}

static int test_mem_profiler_bench_np_0()
{
    benchmark_conf_t conf = get_conf_np();
    conf.mem_conf.enabled = 1;
    conf.mem_conf.poll_time = 1;

    // Run the bench
    benchmark_profile_t output_profile;
    ASSERT(benchmark_program(&conf, &output_profile));
    ASSERT(output_profile.entries != NULL);
    lprintf(LOG_INFO, "example_func_np time is %lu us\n", output_profile.entries->cpu_time_us);

    ASSERT(output_profile.len == 1);
    ASSERT(output_profile.entries->cpu_time_us > 0);

    // Print debug info
    long mem = output_profile.entries[0].max_mem_usage;
    lprintf(LOG_INFO, "There are %lu B\t%lu KiB\t%lu MiB\n", mem, mem / 1024, mem / (1024 * 1024));
    ASSERT(output_profile.entries[0].max_mem_usage == 0);

    free_benchmark_profile(&output_profile);
    return 1;
}

static int test_output_monitor_bench_p()
{
    benchmark_conf_t conf = get_conf_p();
    conf.monitor_func_output = 1;

    // Run the bench
    benchmark_profile_t output_profile;
    ASSERT(benchmark_program(&conf, &output_profile));
    ASSERT(output_profile.entries != NULL);
    ASSERT(output_profile.len == LEN_EXPECTED_P);

    lprintf(LOG_INFO, "example_func_p time is %lu us\n", output_profile.entries[LEN_EXPECTED_P - 1].cpu_time_us);

    long t = 0;
    for (size_t i = 0; i < output_profile.len; i++) {
        t += output_profile.entries[i].cpu_time_us;
        ASSERT(output_profile.entries[i].run_outputs_len == conf.runs_to_average);
        ASSERT(output_profile.entries[i].run_outputs != NULL);
        for (size_t j = 0; j < output_profile.entries[i].run_outputs_len; j++) {
            ASSERT(output_profile.entries[i].run_outputs[j] == 1);
        }
    }
    ASSERT(t > 0);

    free_multi_dimensional_range(&conf.param_conf.params_generator);
    free_benchmark_profile(&output_profile);
    return 1;
}

static int test_output_monitor_bench_np()
{
    benchmark_conf_t conf = get_conf_np();
    conf.monitor_func_output = 1;

    // Run the bench
    benchmark_profile_t output_profile;
    ASSERT(benchmark_program(&conf, &output_profile));
    ASSERT(output_profile.entries != NULL);
    lprintf(LOG_INFO, "example_func_np time is %lu us\n", output_profile.entries->cpu_time_us);

    ASSERT(output_profile.len == 1);
    ASSERT(output_profile.entries->cpu_time_us > 0);

    ASSERT(output_profile.entries->run_outputs_len == conf.runs_to_average);
    ASSERT(output_profile.entries->run_outputs != NULL);
    for (size_t j = 0; j < output_profile.entries->run_outputs_len; j++) {
        ASSERT(output_profile.entries->run_outputs[j] == 1);
    }

    // Print debug info
    long mem = output_profile.entries[0].max_mem_usage;
    lprintf(LOG_INFO, "There are %lu B\t%lu KiB\t%lu MiB\n", mem, mem / 1024, mem / (1024 * 1024));
    ASSERT(output_profile.entries[0].max_mem_usage == 0);

    free_benchmark_profile(&output_profile);
    return 1;
}

SUB_TEST(test_bench, {&test_cpu_time_bench_np, "Test CPU time bench NO PARAMS"},
{&test_cpu_time_bench_p, "Test CPU time bench PARAMS"},
{&test_output_conf, "Test output conf init and free"},
{&test_mem_profiler_bench_p_0, "Test  memory profiler PARAMS no alloc"},
{&test_mem_profiler_bench_np_0, "Test memory profiler NO PARAMS no alloc"},
{&test_output_monitor_bench_p, "Test output moinitoring PARAMS"},
{&test_output_monitor_bench_np, "Test output monitoring NO PARAMS"})
