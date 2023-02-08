#include "./bench_output.h"
#include "./testing.h/logger.h"
#include <stdio.h>
#include <string.h>
#include <jansson.h>

#define JSON_ASSERT(x) if (x != 0) {lprintf(LOG_ERROR, "JSON error\n"); return 0;}
#define NULL_ASSERT(x) if (x == NULL) {lprintf(LOG_ERROR, "JSON error\n"); return 0;}

static int save_benchmark_json_node(benchmark_profile_entry_t entry, json_t *arr)
{
    json_t *vector_node = json_array();
    NULL_ASSERT(vector_node);
    for (size_t i = 0; i < entry.params.dimensions; i++) {
        json_t *val = json_real(entry.params.values[i]);
        NULL_ASSERT(val);
        JSON_ASSERT(json_array_append_new(vector_node, val));
    }

    json_t *run_outputs_node = json_array();
    NULL_ASSERT(run_outputs_node);
    for (size_t i = 0; i < entry.run_outputs_len; i++) {
        json_t *val = json_integer(entry.run_outputs[i]);
        NULL_ASSERT(val);
        JSON_ASSERT(json_array_append_new(run_outputs_node, val));
    }

    json_t *node = json_pack("{so so si si si}",
                             "params", vector_node,
                             "run_outputs", run_outputs_node,
                             "cpu_time_us", entry.cpu_time_us,
                             "cpu_core_time_us", entry.cpu_core_time_us,
                             "max_mem_usage", entry.max_mem_usage);

    NULL_ASSERT(node);
    JSON_ASSERT(json_array_append_new(arr, node));
    return 1;
}

/// Save when NO_PARAMS
static int __save_benchmark_json(benchmark_profile_t *profile, benchmark_output_conf_t *output_conf, FILE *f)
{
    json_t *arr = json_array();
    NULL_ASSERT(arr);

    for (size_t i = 0; i < profile->len; i++) {
        if (!save_benchmark_json_node(profile->entries[i], arr)) {
            lprintf(LOG_ERROR, "Cannot create json, aborting\n");
            return 0;
        }
    }

    JSON_ASSERT(json_dumpf(arr, f, JSON_COMPACT));
    json_decref(arr);
    return 1;
}

int save_benchmark_json(benchmark_profile_t *profile, benchmark_output_conf_t *output_conf)
{
    char name[255];
    snprintf(name, sizeof(name), "%s.bench.json", output_conf->output_file_prefix);

    FILE *f = fopen(name, "w");
    if (f == NULL) {
        lprintf(LOG_ERROR, "Cannot open output file %s\n", name);
        return 0;
    }

    int r, flag = 0;
    switch (profile->conf.function_type) {
    case FUNC_PARAM:
    case FUNC_NO_PARAM:
        r = __save_benchmark_json(profile, output_conf, f);
        flag = 1;
        break;
    }

    fclose(f);
    if (flag) {
        return r;
    }

    lprintf(LOG_ERROR, "Canot find output method for function type\n");
    return 0;
}

static void print_csv_headers(FILE *f, benchmark_profile_t *profile)
{
    for (size_t i = 0; i < profile->conf.param_conf.params_generator.dimensions; i++) {
        fprintf(f, "v%ld,", i);
    }
    fprintf(f, "cpu_time_us,cpu_core_time_us,max_mem_usage,run_outputs\n");
}

static void print_csv_entry(FILE *f, benchmark_profile_t *profile, int i)
{
    fprintf(f, "%lu,%lu,%lu", profile->entries[i].cpu_time_us,
            profile->entries[i].cpu_core_time_us,
            profile->entries[i].max_mem_usage);

    for (size_t j = 0; j < profile->entries[i].run_outputs_len; j++) {
        fprintf(f, ",%d", profile->entries[i].run_outputs[j]);
    }
    fprintf(f, "\n");
    fflush(f);
}

/// Save when PARAMS
static int save_benchmark_csv_p(benchmark_profile_t *profile, benchmark_output_conf_t *output_conf, FILE *f)
{
    print_csv_headers(f, profile);

    for (size_t i = 0; i < profile->len; i++) {
        for (size_t j = 0; j < profile->entries[i].params.dimensions; j++) {
            fprintf(f, "%lf,", profile->entries[i].params.values[j]);
        }
        print_csv_entry(f, profile, i);
    }
    return 1;
}

/// Save when NO_PARAMS
static int save_benchmark_csv_np(benchmark_profile_t *profile, benchmark_output_conf_t *output_conf, FILE *f)
{
    print_csv_headers(f, profile);

    for (size_t i = 0; i < profile->len; i++) {
        print_csv_entry(f, profile, i);
    }
    return 1;
}

int save_benchmark_csv(benchmark_profile_t *profile, benchmark_output_conf_t *output_conf)
{
    char name[255];
    snprintf(name, sizeof(name), "%s.bench.csv", output_conf->output_file_prefix);

    FILE *f = fopen(name, "w");
    if (f == NULL) {
        lprintf(LOG_ERROR, "Cannot open output file %s\n", name);
        return 0;
    }

    int r, flag = 0;
    switch (profile->conf.function_type) {
    case FUNC_PARAM:
        r = save_benchmark_csv_p(profile, output_conf, f);
        flag = 1;
        break;
    case FUNC_NO_PARAM:
        r = save_benchmark_csv_np(profile, output_conf, f);
        flag = 1;
        break;
    }

    fclose(f);
    if (flag) {
        return r;
    }

    lprintf(LOG_ERROR, "Canot find output method for function type\n");
    return 0;
}
