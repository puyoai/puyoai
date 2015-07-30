#ifndef BASE_RANDOM_H_
#define BASE_RANDOM_H_

#include <cstdint>
#include <limits>

// RandomGenerator uses XorShift128+ algorithm.  This algorithm is the fastest known good
// algorithm, and its loop length is 2^128-1. It is long enough for puyoai.
// http://xorshift.di.unimi.it/
class RandomGenerator {
public:
    RandomGenerator();

    // Returns an integer in [0, 2^64).
    std::uint64_t GetUInt64() { return Get(); }

    // Returns an integer in [0, 2^63).  Non-negative only.
    std::int64_t GetInt64() { return Get() & std::numeric_limits<std::int64_t>::max(); }

    // Returns an integer in [0, 2^32).
    std::uint32_t GetUInt32() { return Get() & std::numeric_limits<std::uint32_t>::max(); }

    // Returns an integer in [0, 2^31).  Non-negative only.
    std::int32_t GetInt32() { return Get() & std::numeric_limits<std::int32_t>::max(); }

    // Returns a real number in [0, 1).
    double GetDouble() { return Get() * (1.0 / (std::numeric_limits<std::uint64_t>::max() + 1.0)); }

    // Set random seed(s).
    void Seed(std::uint64_t s);
    void Seed(std::uint64_t* s);

private:
    std::uint64_t Get();

    std::uint64_t seed_[2];
};

namespace random {

// static interfaces
std::uint64_t GetUInt64();
std::int64_t GetInt64();
std::uint32_t GetUInt32();
std::int32_t GetInt32();
double GetDouble();
void Seed(std::uint64_t s);
void Seed(std::uint64_t* s);

}  // namespace random

#endif  // BASE_RANDOM_H_
