#include "./testing.h/testing.h"
#include "./test_mem_profiler.h"
#include "./mem_profiler.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BLOCKS 1024
#define BLOCK_SIZE 1024

int test_memory_profiler()
{
    memory_profiler_t mpt;
    mpt.poll_time = 1;

    ASSERT(init_memory_profiler(&mpt));

    char * blocks[BLOCKS];
    for (size_t i = 0; i < BLOCKS; i++) {
        blocks[i] = malloc(BLOCK_SIZE);
        ASSERT(blocks[i] != NULL);
        for (size_t j = 0; j < BLOCK_SIZE; j++) {
            blocks[i][j] = j;
        }
    }

    usleep(mpt.poll_time * 100);
    size_t usage = max_mem_usage(&mpt);
    lprintf(LOG_INFO, "Currently using about %lu bytes\n", usage);
    ASSERT(usage >= BLOCKS * BLOCK_SIZE);

    calibrate_memory_profiler(&mpt);
    ASSERT(max_mem_usage(&mpt) == 0);

    for (size_t i = 0; i < BLOCKS; i++) {
        free(blocks[i]);
    }

    free_memory_profiler(&mpt);

    return 1;
}
