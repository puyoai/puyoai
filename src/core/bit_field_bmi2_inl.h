#ifndef CORE_BIT_FIELD_BMI2_INL_256_H_
#define CORE_BIT_FIELD_BMI2_INL_256_H_

#ifndef __BMI2__
# error "Needs BMI2 to use this header."
#endif

#include "field_bits_256.h"

inline
void BitField::dropFastAfterVanishBMI2(FieldBits erased)
{
    // TODO(mayah): This method is not so optimized yet. We might have a large room to
    // improve this function.

    union Decomposer64 {
        std::uint64_t v[2];
        __m128i m;
    };

    union Decomposer16 {
        std::uint16_t v[8];
        __m128i m;
    };

    const FieldBits fieldMask = _mm_set_epi16(0, 0x3FFE, 0x3FFE, 0x3FFE, 0x3FFE, 0x3FFE, 0x3FFE, 0);
    const FieldBits leftBits = fieldMask.notmask(erased);
    Decomposer64 x;
    x.m = leftBits;
    const std::uint64_t oldLowBits = x.v[0];
    const std::uint64_t oldHighBits = x.v[1];

    Decomposer16 height;
    height.m = sse::mm_popcnt_epi16(leftBits);

    const std::uint64_t newLowBits =
        (((1ULL << height.v[1]) - 1) << 17) |
        (((1ULL << height.v[2]) - 1) << 33) |
        (((1ULL << height.v[3]) - 1) << 49);
    const std::uint64_t newHighBits =
        (((1ULL << height.v[4]) - 1) << 1) |
        (((1ULL << height.v[5]) - 1) << 17) |
        (((1ULL << height.v[6]) - 1) << 33);

    for (int i = 0; i < 3; ++i) {
        Decomposer64 d;
        d.m = m_[i];

        std::uint64_t extLow = _pext_u64(d.v[0], oldLowBits);
        std::uint64_t depLow = _pdep_u64(extLow, newLowBits);

        std::uint64_t extHigh = _pext_u64(d.v[1], oldHighBits);
        std::uint64_t depHigh = _pdep_u64(extHigh, newHighBits);

        d.v[0] = depLow;
        d.v[1] = depHigh;
        m_[i] = d.m;
    }
}

#endif // CORE_BIT_FIELD_BMI2_INL_256_H_
