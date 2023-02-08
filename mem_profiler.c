#include "./mem_profiler.h"
#include "./testing.h/logger.h"
#include <malloc.h>
#include <unistd.h>

/// A function to make sure that all profiling actions use the same method
static size_t get_malloc_info()
{
    struct mallinfo2 info = mallinfo2();
    return info.uordblks;
}

static void *memory_profiler_thread(void *mpt_raw)
{
    memory_profiler_t *mpt = (memory_profiler_t *) mpt_raw;
    pthread_mutex_unlock(&mpt->lock);

    int flag = 1;
    while (flag) {
        pthread_mutex_lock(&mpt->lock);
        flag = mpt->running;

        size_t usage = get_malloc_info();
        if (usage > mpt->max_mem_usage) {
            mpt->max_mem_usage = usage;
        }
        long period = mpt->poll_time;
        pthread_mutex_unlock(&mpt->lock);

        if (flag) {
            usleep(period);
        }
    }

    pthread_exit(NULL);
    return NULL;
}

int init_memory_profiler(memory_profiler_t *mpt)
{
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    mpt->lock = lock;
    mpt->running = 1;
    mpt->max_mem_usage = 0;

    pthread_mutex_lock(&mpt->lock);
    int s = pthread_create(&mpt->thread, NULL, &memory_profiler_thread, (void *) mpt);
    if (s != 0) {
        lprintf(LOG_ERROR, "Cannot start memory profiler thread\n");
        return 0;
    }
    /*
     The thread will start by unlocking the mutex, this will flag that it has started.
     The calibrate_memory_profiler call is locking meaning that it waits for the mutex to be unlocked by said
     thread start.
     */

    calibrate_memory_profiler(mpt);

    return 1;
}

void free_memory_profiler(memory_profiler_t *mpt)
{
    pthread_mutex_lock(&mpt->lock);
    mpt->running = 0;
    pthread_mutex_unlock(&mpt->lock);

    void *__ret;
    pthread_join(mpt->thread, &__ret);
    pthread_mutex_destroy(&mpt->lock);
}

void calibrate_memory_profiler(memory_profiler_t *mpt)
{
    pthread_mutex_lock(&mpt->lock);
    mpt->start_mem_usage = get_malloc_info();
    mpt->max_mem_usage = mpt->start_mem_usage;
    pthread_mutex_unlock(&mpt->lock);
}

long max_mem_usage(memory_profiler_t *mpt)
{
    long ret;
    pthread_mutex_lock(&mpt->lock);
    if (mpt->max_mem_usage < mpt->start_mem_usage) {
        ret = 0;
        lprintf(LOG_WARNING, "Memory usage is negative, calibration may be wrong\n");
    } else {
        ret = mpt->max_mem_usage - mpt->start_mem_usage;
    }
    pthread_mutex_unlock(&mpt->lock);
    return ret;
}

