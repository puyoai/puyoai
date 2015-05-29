#ifndef BASE_BMI_H_

#include <cstdint>

#ifdef __BMI2__
#include <bmi2intrin.h>
#endif

namespace bmi {

// Same as PEXT instruction.
//      x: HGEFDCBA
//   mask: 01100100
// result: 00000GEC
inline
std::uint64_t extractBits(std::uint64_t x, std::uint64_t mask)
{
#ifdef __BMI2__
    return _pext_u64(x, mask);
#else
    std::uint64_t res = 0;
    for (std::uint64_t bb = 1; mask != 0; bb <<= 1) {
        if (x & mask & -mask)
            res |= bb;
        mask &= (mask - 1);
    }
    return res;
#endif
}

// Same as PDEP instruction.
//      x: HGFEDCBA
//   mask: 01100100
// result: 0CB00A00
inline
std::uint64_t depositBits(std::uint64_t x, std::uint64_t mask)
{
#ifdef __BMI2__
    return _pdep_u64(x, mask);
#else
    std::uint64_t res = 0;
    for (std::uint64_t bb = 1; mask != 0; bb <<= 1) {
        if (x & bb)
            res |= mask & (-mask);
            mask &= (mask - 1);
    }
    return res;
#endif
}

// 4bit version of extractBits.
//      x: DDDD CCCC BBBB AAAA
//   mask:    0    1    0    1
// result: 0000 0000 CCCC AAAA
inline
std::uint64_t extractBits4(std::uint64_t x, int mask)
{
    std::uint64_t m = depositBits(mask, 0x1111111111111111);
    m = m | (m << 1);
    m = m | (m << 2);

    return extractBits(x, m);
}

} // namespace bmi

#endif
