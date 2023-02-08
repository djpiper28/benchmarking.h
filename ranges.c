#include "./ranges.h"
#include "./testing.h/logger.h"
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>

void range_start(range_t *range)
{
    range->current = range->start;
}

range_state_t range_next(range_t *range, double *output)
{
    range->current += range->step;
    if (isgreater(range->current, range->end)) {
        return RANGE_STOPPED;
    }

    *output = range->current;
    return RANGE_GENERATING;
}

void free_vector(vector_t *vect)
{
    if (vect == NULL) return;
    if (vect->values == NULL) return;
    free(vect->values);
}

int init_multi_dimensional_range_arr(multi_dimensional_range_t *range, size_t d, range_t *items)
{
    range->ranges = malloc(sizeof(*range->ranges) * d);
    if (range->ranges == NULL) {
        lprintf(LOG_ERROR, "Cannot malloc in mdd range init\n");
        return 0;
    }

    for (size_t i = 0; i < d; i++) {
        range->ranges[i] = items[i];
    }

    range->dimensions = d;
    return 1;
}

int __va_args_init_multi_dimensional_range(multi_dimensional_range_t *range, size_t d, range_t one, ...)
{
    va_list ap;
    va_start(ap, one);

    size_t len = d + 1, ptr = 0;
    range->ranges = malloc(sizeof(*range->ranges) * len);
    if (range->ranges == NULL) {
        va_end(ap);
        lprintf(LOG_ERROR, "Cannot malloc in mdd range init\n");
        return 0;
    }

    range->ranges[ptr++] = one;
    for (size_t i = 0; i < d; i++) {
        range->ranges[ptr++] = va_arg(ap, range_t);
    }

    range->dimensions = len;

    va_end(ap);
    return 1;
}

void multi_dimensional_range_start(multi_dimensional_range_t *range)
{
    for (size_t i = 0; i < range->dimensions; i++) {
        range_start(&range->ranges[i]);
    }

    // Offset for the algorithm to work
    size_t i = range->dimensions - 1;
    range->ranges[i].current -= range->ranges[i].step;
}

void free_multi_dimensional_range(multi_dimensional_range_t *range)
{
    if (range == NULL) return;
    if (range->ranges != NULL)
        free(range->ranges);
}

range_state_t multi_dimensional_range_next(multi_dimensional_range_t *range, vector_t *output)
{
    // Iterate the last dimension, when reset then repeat for the next dimension
    long ptr = range->dimensions - 1;
    output->dimensions = 0;

    range_state_t ret = RANGE_GENERATING;
    int flag = 1;
    while (flag) {
        if (ptr < 0) {
            flag = 0;
            ret = RANGE_STOPPED;
        } else {
            double __val; // Not used
            range_state_t state = range_next(&range->ranges[ptr], &__val);
            if (state == RANGE_STOPPED) {
                range_start(&range->ranges[ptr]);
                ptr--;
            } else {
                flag = 0;
            }
        }
    }

    if (ret == RANGE_STOPPED) {
        output->values = NULL;
        return ret;
    }

    output->values = malloc(sizeof(*output->values) * range->dimensions);
    if (output->values == NULL) {
        lprintf(LOG_ERROR, "Fatal: Cannot malloc a range\n");
        return RANGE_ERROR;
    }

    for (size_t i = 0; i < range->dimensions; i++) {
        output->values[i] = range->ranges[i].current;
    }
    output->dimensions = range->dimensions;

    return ret;
}
