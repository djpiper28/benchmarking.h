#include "./time_utils.h"
#include <stdlib.h>

long time_diff(struct timeval x, struct timeval y)
{
    // Source https://www.gnu.org/software/libc/manual/html_node/Calculating-Elapsed-Time.html
    /* Perform the carry for the later subtraction by updating y. */
    if (x.tv_usec < y.tv_usec) {
        int nsec = (y.tv_usec - x.tv_usec) / 1000000 + 1;
        y.tv_usec -= 1000000 * nsec;
        y.tv_sec += nsec;
    }
    if (x.tv_usec - y.tv_usec > 1000000) {
        int nsec = (x.tv_usec - y.tv_usec) / 1000000;
        y.tv_usec += 1000000 * nsec;
        y.tv_sec -= nsec;
    }

    /* Compute the time remaining to wait.
       tv_usec is certainly positive. */
    long tv_sec = x.tv_sec - y.tv_sec;
    long tv_usec = x.tv_usec - y.tv_usec;

    return labs(tv_usec + (1000 * 1000) * tv_sec);
}
