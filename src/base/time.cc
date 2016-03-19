#include "base/time.h"

#include <chrono>

using namespace std::chrono;

namespace {

using Clock = high_resolution_clock;

Clock::duration durationFromBase()
{
    static Clock::time_point s_clockBase = Clock::now();
    return Clock::now() - s_clockBase;
}

}

double currentTime()
{
    return duration_cast<duration<double>>(durationFromBase()).count();
}

std::int64_t currentTimeInMillis()
{
    return duration_cast<milliseconds>(durationFromBase()).count();
}
