#pragma once
#include <sys/time.h>

/// The difference in time between a and, b
/// returns b - a in micro seconds (us)
long time_diff(struct timeval a, struct timeval b);

