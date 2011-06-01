#include "common.h"

#include "timer.h"

uint64 Timer::tick()
{
    return GetTickCount64();
}

void Timer::sleep(uint32 ms)
{
    Sleep(ms);
}
