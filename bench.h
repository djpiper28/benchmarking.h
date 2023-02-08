#pragma once
#include "./ranges.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Memory profile settings
typedef struct benchmark_mem_conf_t {
    /// whether to profile memory
    int enabled;
    /// ms, recommended is about 1
    long poll_time;
} benchmark_mem_conf_t;

/// The default config for memory profiling
#define DEFAULT_BENCHMARK_MEM_CONF {1, 1}

/// CPU profile settings, this looks at all cores and,
/// is probably better than CPU time.
typedef struct benchmark_cpu_conf_t {
    /// Whether to profile cpu usage
    int enabled;
    /// ms, recommended is about 2500
    long poll_time;
} benchmark_cpu_conf_t;

/// The default config for cpu core profiling
#define DEFAULT_BENCHMARK_CPU_CONF {1, 2500}

typedef enum benchmark_func_type_t {
    FUNC_PARAM,
    FUNC_NO_PARAM
} benchmark_func_type_t;

/// Configuration for parameters for the function that is called
/// this allows for exciting data to be generated.
typedef struct benchmark_param_conf_t {
    multi_dimensional_range_t params_generator;
} benchmark_param_conf_t;

/// Configuration for the benchmark
typedef struct benchmark_conf_t {
    /// Number of runs of the benchmark to do to get an average
    size_t runs_to_average;
    benchmark_cpu_conf_t cpu_conf;
    benchmark_mem_conf_t mem_conf;

    /// If this is set to FUNC_PARAM then param_conf must be set
    benchmark_func_type_t function_type;
    union {
        /// if function_type is FUNC_NO_PARAM, set this to the func to benchmark
        int (*np_func)();
        /// if function type is FUNC_PARAM, set this to the func to benchmark
        /// and set param_conf.
        int (*p_func)(vector_t params);
    };

    /// If FUNC_PARAM this must be set to the generator for the parameters send to p_func
    benchmark_param_conf_t param_conf;

    /// Function output is 0 for failure, toggling this will save output,
    /// allowing for functions to provide data for plotting if you want that
    int monitor_func_output;
} benchmark_conf_t;

/// Different ways for the output to be saved
typedef enum benchmark_output_type_t {
    /// A single json file
    OUTPUT_JSON,
    /// Many CSV files
    OUTPUT_CSV
} benchmark_output_type_t;

/// This struct is the output configuration for benchmarks
typedef struct benchmark_output_conf_t {
    /// This is the prefix for the file that the output is saved as
    char *output_file_prefix;
    /// The type of output (i.e: JSON or, CSV)
    benchmark_output_type_t output_type;
} benchmark_output_conf_t;

/// Inits the config, a NULL name will make a default prefix be used (recommended?)
int init_benchmark_output_conf(benchmark_output_conf_t *conf, benchmark_output_type_t t, char *name);

/// Frees the config
/// This does not free any ranges that are in the conf. these are "owned" by the caller
void free_benchmark_output_conf(benchmark_output_conf_t *conf);

/// A profile will contain many entries, this will store data for a function with certain parameters
/// This is an average for the runs (as specified in the config), everything that is continuous is
/// averaged, run_outputs are not.
typedef struct benchmark_profile_entry_t {
    /// Has length 0 is there are no parameters for the entry.
    vector_t params;
    /// A measure of how much time the benchmark took to complete
    size_t cpu_time_us;
    /// Set to MAX_LONG_INT if this profile is disabled (benchmark_cpu_conf_t)
    size_t cpu_core_time_us;
    /// Set to MAX_LONG_INT if this profile is disabled (benchmark_mem_conf_t)
    size_t max_mem_usage;
    /// The length of outputs used in the run
    size_t run_outputs_len;
    /// The output of all runs, users may want to get the mean, median or, mode later on.
    int *run_outputs;
} benchmark_profile_entry_t;

/// Stores the results of a run
typedef struct benchmark_profile_t {
    size_t len;
    benchmark_profile_entry_t *entries;
    benchmark_conf_t conf;
} benchmark_profile_t;

/// Runs a benchmark from a set of configurations, inputs are not cloned and, owned by the caller
int benchmark_program(benchmark_conf_t *conf_bench, benchmark_profile_t *output_profile);

/// Saves the benchmark results to a file using the output configuration that is passed as a parameter
int save_benchmark(benchmark_profile_t *profile, benchmark_output_conf_t *output_conf);

/// Frees the benchmark profile after it has been generated
/// The pointer that is passed is NOT freed - it is your memory not ours.
void free_benchmark_profile(benchmark_profile_t *profile);

#ifdef __cplusplus
}
#endif

