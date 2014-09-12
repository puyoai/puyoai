#include "base/time_stamp_counter.h"

#include <algorithm>
#include <iostream>
#include <map>

#include <cmath>

using namespace std;

namespace {

unsigned long long rdtsc()
{
    unsigned long long tsc = 0;
#if defined(__i386__)
    asm volatile("rdtsc" : "=A" (tsc));
#elif defined(__x86_64__)
    unsigned int tscl, tsch;
    asm volatile("rdtsc":"=a"(tscl),"=d"(tsch));
    tsc = ((unsigned long long)tsch << 32)| tscl;
#else
# warning "rdtsc() isn't implemented for thie architecture."
#endif
    return tsc;
}

}

void TimeStampCounterData::showStatistics() const
{
    int n = data_.size();
    if (n <= 1) {
        cout << "no enough data to show statistics" << endl;
        return;
    }

    double sum = 0.0;
    for (auto x : data_) {
        sum += x;
    }
    double average = sum / n;

    double diffSquareSum = 0.0;
    for (auto x : data_) {
        diffSquareSum += (x - average) * (x - average);
    }

    double deviation = pow(diffSquareSum / n, 0.5);

    cout << "        N = " << n << endl;
    cout << "      min = " << *min_element(data_.begin(), data_.end()) << endl;
    cout << "      max = " << *max_element(data_.begin(), data_.end()) << endl;
    cout << "  average = " << average << endl;
    cout << "deviation = " << deviation << endl;
}

ScopedTimeStampCounter::ScopedTimeStampCounter(TimeStampCounterData* tsc) :
    tsc_(tsc),
    start_(rdtsc())
{
}

ScopedTimeStampCounter::~ScopedTimeStampCounter()
{
    unsigned long long end = rdtsc();
    if (end > start_)
        tsc_->add(end - start_);
}
