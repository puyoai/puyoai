#ifndef CORE_BIT_FIELD_BMI2_INL_256_H_
#define CORE_BIT_FIELD_BMI2_INL_256_H_

#ifndef __BMI2__
# error "Needs BMI2 to use this header."
#endif

#include "field_bits_256.h"

inline
void BitField::dropAfterVanishFastBMI2(FieldBits erased)
{
    union Decomposer64 {
        std::uint64_t v[2];
        __m128i m;
    };

    union Decomposer256_64 {
        std::uint64_t v[4];
        __m256i m;
    };

    const FieldBits fieldMask = FieldBits::FIELD_MASK_13;
    const FieldBits leftBits = fieldMask.notmask(erased);
    Decomposer64 x;
    x.m = leftBits;
    const std::uint64_t oldLowBits = x.v[0];
    const std::uint64_t oldHighBits = x.v[1];

    const __m256i ones = _mm256_set_epi32(0, 1, 1, 1, 1, 1, 1, 0);
    __m256i height = _mm256_cvtepi16_epi32(sse::mm_popcnt_epi16(leftBits));
    height = _mm256_sllv_epi32(ones, height);
    height = _mm256_sub_epi32(height, ones);
    height = _mm256_slli_epi32(height, 1);

    height = _mm256_packs_epi32(height, height);
    Decomposer256_64 y;
    y.m = height;
    const std::uint64_t newLowBits = y.v[0];
    const std::uint64_t newHighBits = y.v[2];

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
