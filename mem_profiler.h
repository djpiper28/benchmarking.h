#pragma once
#include <pthread.h>

typedef struct memory_profiler_t {
    pthread_t thread;
    pthread_mutex_t lock;
    size_t start_mem_usage;
    size_t max_mem_usage;
    size_t poll_time;
    int running;
} memory_profiler_t;

/// Inits and, starts the memory profiler, returning when the thread is active
int init_memory_profiler(memory_profiler_t *mpt);

/// Join and, frees a profiler.
void free_memory_profiler(memory_profiler_t *mpt);

/// Sets the start_mem_usage to allow for results to ignore the current heap size
void calibrate_memory_profiler(memory_profiler_t *mpt);

/// Thread safe getter for the max memory usage
long max_mem_usage(memory_profiler_t *mpt);

