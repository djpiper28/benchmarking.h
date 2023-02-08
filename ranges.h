#pragma once
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Single dimensional iterater range
typedef struct range_t {
    double start, end, step;

    // Internal generator state
    double current;
} range_t;

#define RANGE_T_DEFAULT {0, 0, 0, 0}

/// State of the range iteration
typedef enum range_state_t {
    RANGE_GENERATING = 1,
    RANGE_STOPPED = 0,
    RANGE_ERROR = -1
} range_state_t;

/// Resets the internal state allowing for the range to be (re)started
void range_start(range_t *range);
/// Gets the next value from a range, this modifies its internal state
range_state_t range_next(range_t *range,
                         double *output);

/// Multi dimensional ranges struct, use an init_multi_dimensional method
typedef struct multi_dimensional_range_t {
    size_t dimensions;
    range_t *ranges;
} multi_dimensional_range_t;

/// Output for an iteration of a multi_dimensional_range_t
typedef struct vector_t {
    size_t dimensions;
    double *values;
} vector_t;

void free_vector(vector_t *vect);

#define NUMARGS(t, ...)  (sizeof((t[]){__VA_ARGS__})/sizeof(t))

/// All args must be of type (range_t), 0 on failure,
/// use the init_multi_dimensional_range macro to have a specified size
int init_multi_dimensional_range_arr(multi_dimensional_range_t *range,
                                     size_t d,
                                     range_t *items);

/// Use init_multi_dimensional as a wrapper to set d automatically
int __va_args_init_multi_dimensional_range(multi_dimensional_range_t *range,
        size_t d,
        range_t one,
        ...);

/// A macro wrapper for __init_multi_dimensional_range to specify the size
#define init_multi_dimensional_range(range, one, ...) __va_args_init_multi_dimensional_range(range, NUMARGS(range_t, __VA_ARGS__), one, __VA_ARGS__)

/// Frees a range, call even on error.
void free_multi_dimensional_range(multi_dimensional_range_t *range);

/// Resets the internal state allowing for the range to be (re)started
void multi_dimensional_range_start(multi_dimensional_range_t *range);
/// Gets the next value from a range, this modifies its internal state,
/// output is heap allocated, please free_vector if not null.
range_state_t multi_dimensional_range_next(multi_dimensional_range_t *range,
        vector_t *output);

#ifdef __cplusplus
}
#endif

