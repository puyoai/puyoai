#include "base/random.h"

#include <cstdint>

RandomGenerator::RandomGenerator() {
    seed_[0] = 0xa197b12e68a9d197ULL;
    seed_[1] = 0x86f5e89c71d657d8ULL;
}

std::uint64_t RandomGenerator::Get() {
    std::uint64_t s1 = seed_[0];
    const std::uint64_t s0 = seed_[1];
    seed_[0] = s0;
    s1 ^= s1 << 23;
    seed_[1] = (s1 ^ s0 ^ (s1 >> 17) ^ (s0 >> 26));
    return seed_[1] + s0;
}

void RandomGenerator::Seed(std::uint64_t s) {
    seed_[0] = s;
    seed_[1] = 0x8c91e375a83a47e8ULL;
}

void RandomGenerator::Seed(std::uint64_t* s) {
    seed_[0] = s[0];
    seed_[1] = s[1];
}

namespace {

RandomGenerator* prg_ = nullptr;

RandomGenerator* PRG() {
    if (!prg_)
        prg_ = new RandomGenerator;
    return prg_;
}

}  // namespace

namespace random {

std::uint64_t GetUInt64() {
    return PRG()->GetUInt64();
}

std::int64_t GetInt64() {
    return PRG()->GetInt64();
}

std::uint32_t GetUInt32() {
    return PRG()->GetUInt32();
}

std::int32_t GetInt32() {
    return PRG()->GetInt32();
}

double GetDouble() {
    return PRG()->GetDouble();
}

void Seed(std::uint64_t s) {
    PRG()->Seed(s);
}

void Seed(std::uint64_t* s) {
    PRG()->Seed(s);
}

}  // namespace random
