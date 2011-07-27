#include "timer.h"

#include "common.h"

using namespace trassel;

uint64 Timer::tick()
{
    return GetTickCount64();
}

void Timer::sleep(uint32 ms)
{
    Sleep(ms);
}
