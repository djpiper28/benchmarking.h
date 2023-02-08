#include "./bench.h"
#include "./bench_output.h"
#include "./testing.h/logger.h"
#include "./time_utils.h"
#include "./mem_profiler.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>

int init_benchmark_output_conf(benchmark_output_conf_t *conf, benchmark_output_type_t t, char *name)
{
    if (name == NULL) {
        // Get time
        time_t rawtime;
        time(&rawtime);
        struct tm *info = localtime(&rawtime);

        size_t len = 256;
        conf->output_file_prefix = malloc(sizeof(*conf->output_file_prefix) * len);
        if (conf->output_file_prefix == NULL) {
            lprintf(LOG_ERROR, "Cannot malloc prefix\n");
            return 0;
        }

        // Time to string
        strftime(conf->output_file_prefix, len, "%x - %H:%M:%S %Z", info);
        strncat(conf->output_file_prefix, ".benchmark.", len - 1);
    } else {
        size_t len = strlen(name) + 1;
        conf->output_file_prefix = malloc(sizeof(*conf->output_file_prefix) * len);
        if (conf->output_file_prefix == NULL) {
            lprintf(LOG_ERROR, "Cannot malloc prefix\n");
            return 0;
        }

        strcpy(conf->output_file_prefix, name);
    }

    conf->output_type = t;
    return 1;
}

void free_benchmark_output_conf(benchmark_output_conf_t *conf)
{
    if (conf == NULL) return;
    if (conf->output_file_prefix != NULL) {
        free(conf->output_file_prefix);
    }
}

int benchmark_program(benchmark_conf_t *conf_bench, benchmark_profile_t *output_profile)
{
    // Init output
    output_profile->conf = *conf_bench;
    output_profile->len = 0;
    output_profile->entries = malloc (sizeof(*output_profile->entries));
    if (output_profile->entries == NULL) {
        lprintf(LOG_ERROR, "Cannot malloc entries\n");
        return 0;
    }

    memory_profiler_t mtp;
    if (conf_bench->mem_conf.enabled) {
        mtp.poll_time = conf_bench->mem_conf.poll_time;
        init_memory_profiler(&mtp);
    }

    // Run the benchmark runs
    // Run function with no paramas if needed
    if (conf_bench->function_type == FUNC_NO_PARAM) {
        // Run the benchmark
        struct timeval start;
        gettimeofday(&start, NULL);
        memset(output_profile->entries, 0, sizeof(*output_profile->entries));

        if (conf_bench->monitor_func_output) {
            output_profile->entries->run_outputs = malloc(sizeof(*output_profile->entries->run_outputs) * conf_bench->runs_to_average);
            if (output_profile->entries->run_outputs == NULL) {
                lprintf(LOG_ERROR, "Cannot allocate run outputs array\n");
            }

            output_profile->entries->run_outputs_len = conf_bench->runs_to_average;
        }
        // The length for NO_PARAM is always 1
        output_profile->len = 1;

        for (size_t i = 0; i < conf_bench->runs_to_average; i++) {
            // Reset profiler state
            if (conf_bench->mem_conf.enabled) {
                calibrate_memory_profiler(&mtp);
            }

            // Run the benchmark run
            int s = conf_bench->np_func();

            if (conf_bench->mem_conf.enabled) {
                output_profile->entries->max_mem_usage += max_mem_usage(&mtp) / conf_bench->runs_to_average;
            }

            if (conf_bench->monitor_func_output) {
                output_profile->entries->run_outputs[i] = s;
            }
        }

        struct timeval end;
        gettimeofday(&end, NULL);

        long us_diff = time_diff(start, end) / conf_bench->runs_to_average;
        output_profile->entries->cpu_time_us = us_diff;
    }
    // Run function with params otherwsie
    else if (conf_bench->function_type == FUNC_PARAM) {
        // Iterate over the param ranges as applicable
        multi_dimensional_range_start(&conf_bench->param_conf.params_generator);
        vector_t vect;
        while (1) {
            range_state_t state = multi_dimensional_range_next(&conf_bench->param_conf.params_generator, &vect);
            if (state != RANGE_GENERATING) {
                if (state == RANGE_ERROR) {
                    lprintf(LOG_ERROR, "Cannot generate new range\n");
                    return 0;
                }

                // RANGE_STOPPED has been reached meaning there are no more runs needed
                break;
            }

            struct timeval start;
            gettimeofday(&start, NULL);

            // Realloc the entries array
            size_t ptr = output_profile->len;
            output_profile->entries = realloc(output_profile->entries, sizeof(*output_profile->entries) * (ptr + 1));
            if (output_profile->entries == NULL) {
                lprintf(LOG_ERROR, "Cannot realloc entries\n");
                return 0;
            }

            memset(&output_profile->entries[ptr], 0, sizeof(output_profile->entries[output_profile->len]));

            if (conf_bench->monitor_func_output) {
                output_profile->entries[ptr].run_outputs = malloc(sizeof(*output_profile->entries[ptr].run_outputs) * conf_bench->runs_to_average);
                if (output_profile->entries[ptr].run_outputs == NULL) {
                    lprintf(LOG_ERROR, "Cannot allocate run outputs array\n");
                }

                output_profile->entries[ptr].run_outputs_len = conf_bench->runs_to_average;
            }

            // Start profilers
            for (size_t i = 0; i < conf_bench->runs_to_average; i++) {
                // Reset profilers state
                if (conf_bench->mem_conf.enabled) {
                    calibrate_memory_profiler(&mtp);
                }

                // Run the benchmark run
                int s = conf_bench->p_func(vect);

                if (conf_bench->mem_conf.enabled) {
                    output_profile->entries[ptr].max_mem_usage += max_mem_usage(&mtp) / conf_bench->runs_to_average;
                }

                // If recording output state realloc the run_outputs
                if (conf_bench->monitor_func_output) {
                    output_profile->entries[ptr].run_outputs[i] = s;
                }
            }

            struct timeval end;
            gettimeofday(&end, NULL);

            // Get average time
            long us_diff = time_diff(start, end) / conf_bench->runs_to_average;
            output_profile->entries[ptr].cpu_time_us = us_diff;
            output_profile->entries[ptr].params = vect;

            // Continue the iteration
            output_profile->len++;
        }
    } else {
        lprintf(LOG_ERROR, "Invalid function type\n");
    }

    if (conf_bench->mem_conf.enabled) {
        free_memory_profiler(&mtp);
    }

    return 1;
}

int save_benchmark(benchmark_profile_t *profile, benchmark_output_conf_t *output_conf)
{
    // Check output type and, call the bench_output function that is responsible for it
    switch (output_conf->output_type) {
    case OUTPUT_JSON:
        return save_benchmark_json(profile, output_conf);
    case OUTPUT_CSV:
        return save_benchmark_csv(profile, output_conf);
    }

    lprintf(LOG_ERROR, "Cannot find output type\n");
    return 0;
}

void free_benchmark_profile(benchmark_profile_t *profile)
{
    if (profile == NULL) return;
    if (profile->entries != NULL) {
        for (size_t i = 0; i < profile->len; i++) {
            if (profile->entries[i].run_outputs != NULL) {
                free(profile->entries[i].run_outputs);
            }

            free_vector(&profile->entries[i].params);
        }
        free(profile->entries);
    }
}
