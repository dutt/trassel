#include "common.h"

#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include "timer.h"

uint64 Timer::tick()
{
    /* TODO: Replace with clock_gettime(CLOCK_MONOTONIC) if its usable? */
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}

void Timer::sleep(uint32 ms)
{
    /* TODO: Check for signals ? or is it ok if we may return early? */
    struct timespec req, rem;
    req.tv_sec = ms / 1000;
    req.tv_nsec = (ms % 1000) * 1000000l;
    nanosleep(&req, &rem);
}
