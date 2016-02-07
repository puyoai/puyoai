#ifndef BASE_BUILTIN_H_
#define BASE_BUILTIN_H_

#include <cstdint>

// Returns the number of 1-bits in |x|.
inline int popCount32(const std::uint32_t& x)
{
    return __builtin_popcount(x);
}
inline int popCount64(const std::uint64_t& x)
{
    return __builtin_popcountll(x);
}

// Returns the number of leading/trailing 0-bits in x, starting at the MSB/LSB.
// If x is 0, the result is undefined.
// e.g. x = 00101000 10010101 10111000 01000000, CLZ(x) = 2 and CTZ = 6.
//          ^^CLZ counts them.           ^^^^^^CTZ counts them.
inline int countLeadingZeros32(const std::uint32_t& x)
{
    return __builtin_clz(x);
}
inline int countTrailingZeros32(const std::uint32_t& x)
{
    return __builtin_ctz(x);
}
inline int countLeadingZeros64(const std::uint64_t& x)
{
    return __builtin_clzll(x);
}
inline int countTrailingZeros64(const std::uint64_t& x)
{
    return __builtin_ctzll(x);
}

#endif // BASE_BUILTIN_H_

