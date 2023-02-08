#include "./test_bench_output.h"
#include "./bench.h"
#include "./testing.h/testing.h"
#include <math.h>
#include <string.h>

static benchmark_output_conf_t get_output_conf_csv(char *prefix)
{
    benchmark_output_conf_t ret;
    init_benchmark_output_conf(&ret, OUTPUT_CSV, prefix);
    return ret;
}

static benchmark_output_conf_t get_output_conf_json(char *prefix)
{
    benchmark_output_conf_t ret;
    init_benchmark_output_conf(&ret, OUTPUT_JSON, prefix);
    return ret;
}

// Copied from ./test_bench.c
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

    conf.monitor_func_output = 1;
    conf.mem_conf.enabled = 1;
    conf.mem_conf.poll_time = 1;

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

    conf.monitor_func_output = 1;
    conf.mem_conf.enabled = 1;
    conf.mem_conf.poll_time = 1;

    return conf;
}

static int test_csv_output_np()
{
    benchmark_conf_t conf = get_conf_np();

    // Run the bench
    benchmark_profile_t output_profile;
    ASSERT(benchmark_program(&conf, &output_profile));
    ASSERT(output_profile.entries != NULL);
    lprintf(LOG_INFO, "example_func_np time is %lu us\n", output_profile.entries->cpu_time_us);

    ASSERT(output_profile.len == 1);
    ASSERT(output_profile.entries->cpu_time_us > 0);

    benchmark_output_conf_t o_conf = get_output_conf_csv("test_csv_output_np"); // __FUNCTION__ does not exist in ISO C
    ASSERT(save_benchmark(&output_profile, &o_conf));
    free_benchmark_output_conf(&o_conf);

    // Test output
    FILE *f = fopen("test_csv_output_np.bench.csv", "r");
    ASSERT(f != NULL);

    size_t len = RUNS_TO_AVERAGE * 9 + 100;
    char *buffer = malloc(len);
    ASSERT(fgets(buffer, len, f) != NULL); // read headers

    size_t i = 0;
    while (fgets(buffer, len, f) != NULL) {
        size_t c = 0;
        for (size_t j = 0; j < len && buffer[j] != 0; j++) {
            if (buffer[j] == ',') {
                c++;
            }
        }

        // ASSERT there are enough commas
        ASSERT(c == 3 + conf.runs_to_average - 1);
        i++;
    }
    free(buffer);

    // ASSERT there are the correct amount of lines
    ASSERT(i == output_profile.len);
    fclose(f);

    free_benchmark_profile(&output_profile);
    return 1;
}

static int test_csv_output_p()
{
    benchmark_conf_t conf = get_conf_p();

    // Run the bench
    benchmark_profile_t output_profile;
    ASSERT(benchmark_program(&conf, &output_profile));
    ASSERT(output_profile.entries != NULL);
    lprintf(LOG_INFO, "example_func_p time is %lu us\n", output_profile.entries->cpu_time_us);

    ASSERT(output_profile.len == LEN_EXPECTED_P);
    ASSERT(output_profile.entries->cpu_time_us > 0);

    benchmark_output_conf_t o_conf = get_output_conf_csv("test_csv_output_p");
    ASSERT(save_benchmark(&output_profile, &o_conf));
    free_benchmark_output_conf(&o_conf);

    // Test output
    FILE *f = fopen("test_csv_output_p.bench.csv", "r");
    ASSERT(f != NULL);

    size_t len = RUNS_TO_AVERAGE * 9 + 100;
    char *buffer = malloc(len);
    ASSERT(fgets(buffer, len, f) != NULL); // read headers

    size_t i = 0;
    while (fgets(buffer, len, f) != NULL) {
        size_t c = 0;
        for (size_t j = 0; j < len && buffer[j] != 0; j++) {
            if (buffer[j] == ',') {
                c++;
            }
        }

        // ASSERT there are enough commas
        ASSERT(c == 3 + conf.runs_to_average - 1 + conf.param_conf.params_generator.dimensions);
        i++;
    }
    free(buffer);

    // ASSERT there are the correct amount of lines
    ASSERT(i == output_profile.len);
    fclose(f);

    free_benchmark_profile(&output_profile);
    return 1;
}

static int test_json_output_p()
{
    benchmark_conf_t conf = get_conf_p();

    // Run the bench
    benchmark_profile_t output_profile;
    ASSERT(benchmark_program(&conf, &output_profile));
    ASSERT(output_profile.entries != NULL);
    lprintf(LOG_INFO, "example_func_p time is %lu us\n", output_profile.entries->cpu_time_us);

    ASSERT(output_profile.len == LEN_EXPECTED_P);
    ASSERT(output_profile.entries->cpu_time_us > 0);

    benchmark_output_conf_t o_conf = get_output_conf_json("test_json_output_p");
    ASSERT(save_benchmark(&output_profile, &o_conf));
    free_benchmark_output_conf(&o_conf);
    free_benchmark_profile(&output_profile);

    return 1;
}

static int test_json_output_np()
{
    benchmark_conf_t conf = get_conf_np();

    // Run the bench
    benchmark_profile_t output_profile;
    ASSERT(benchmark_program(&conf, &output_profile));
    ASSERT(output_profile.entries != NULL);
    lprintf(LOG_INFO, "example_func_p time is %lu us\n", output_profile.entries->cpu_time_us);

    ASSERT(output_profile.len == 1);
    ASSERT(output_profile.entries->cpu_time_us > 0);

    benchmark_output_conf_t o_conf = get_output_conf_json("test_json_output_np");
    ASSERT(save_benchmark(&output_profile, &o_conf));
    free_benchmark_output_conf(&o_conf);
    free_benchmark_profile(&output_profile);

    return 1;
}

SUB_TEST(test_bench_output, {&test_csv_output_np, "Test CSV output NO PARAMS"},
{&test_csv_output_p, "Test CSV output PARAMS"},
{&test_json_output_p, "Test JSON output PARAMS"},
{&test_json_output_np, "Test  JSON output NO PARAMS"})
