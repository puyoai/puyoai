#ifndef BASE_TIME_STAMP_COUNTER_H_
#define BASE_TIME_STAMP_COUNTER_H_

#include <vector>

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
    explicit ScopedTimeStampCounter(TimeStampCounterData* tsc);
    ~ScopedTimeStampCounter();

private:
    TimeStampCounterData* tsc_;
    unsigned long long start_;
};

#endif
