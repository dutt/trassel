#ifndef TIMER_HPP
#define TIMER_HPP

#include "typedefs.h"

class Timer
{
public:
    static uint64 tick();

    static void sleep(uint32 ms);
};

#endif /* TIMER_HPP */
