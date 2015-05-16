#ifndef BASE_TIME_STAMP_COUNTER_H_
#define BASE_TIME_STAMP_COUNTER_H_

#include <vector>
#include <x86intrin.h>

class TimeStampCounterData {
public:
    void showStatistics() const;
    void add(unsigned long long t) { data_.push_back(t); }

private:
    std::vector<unsigned long long> data_;
};

// Precision of this timer depends on the CPU clock of the machine running on.
class ScopedTimeStampCounter {
public:
    // Don't take the ownership of tsc
    explicit ScopedTimeStampCounter(TimeStampCounterData* tsc) :
        tsc_(tsc),
        start_(__rdtscp(&a_))
    {
    }

    ~ScopedTimeStampCounter()
    {
        unsigned long long end = __rdtscp(&a_);
        if (end > start_)
            tsc_->add(end - start_);
    }

private:
    TimeStampCounterData* tsc_;
    unsigned int a_;
    unsigned long long start_;
};

#endif
