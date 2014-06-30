#include "util.h"

#include <sys/time.h>

double now()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}
