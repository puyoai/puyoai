#ifndef BASE_SSE_H_
#define BASE_SSE_H_

#include <smmintrin.h>
#include <cstdint>

namespace sse {

union Decomposer {
    __m128i m;
    std::uint64_t ui64[2];
    std::uint32_t ui32[4];
    std::uint16_t ui16[8];
    std::uint8_t ui8[16];
};

// Makes __m128i x s.t. mask = _mm_movemask_epi8(x).
inline __m128i inverseMovemask(int mask)
{
    // insert
    // 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    //                                           mH mL

    __m128i vmask = _mm_insert_epi16(_mm_setzero_si128(), mask, 0);

    // shuffle
    // 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    // mH mH mH mH mH mH mH mH mL mL mL mL mL mL mL mL

    const __m128i shuffle(_mm_set_epi32(0x01010101, 0x01010101, 0, 0));
    vmask = _mm_shuffle_epi8(vmask, shuffle);

    // or
    // 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
    // mH mH mH mH mH mH mH mH mL mL mL mL mL mL mL mL
    // 7f bf df ef f7 fb fd fe 7f bf df ef f7 fb fd fe

    const __m128i bit_mask(_mm_set_epi32(0x7fbfdfef, 0xf7fbfdfe, 0x7fbfdfef, 0xf7fbfdfe));
    vmask = _mm_or_si128(vmask, bit_mask);

    // compare with -1.
    return _mm_cmpeq_epi8(vmask, _mm_set1_epi8(-1));
}

// Returns __m128i where all bits are set to 1.
inline __m128i mm_setone_si128()
{
    __m128i zeros = _mm_setzero_si128();
    return _mm_cmpeq_epi64(zeros, zeros);
}

// Bit-wise not for __m128i.
inline __m128i mm_not_si128(__m128i x)
{
    return _mm_xor_si128(mm_setone_si128(), x);
}

// Parallel bit-wise or operation for each 16 bits.
// 0001xxxxxxxxxxxx --> 0001111111111111
inline __m128i mm_porr_epi16(__m128i x)
{
    x = _mm_or_si128(x, _mm_srli_epi16(x, 1));
    x = _mm_or_si128(x, _mm_srli_epi16(x, 2));
    x = _mm_or_si128(x, _mm_srli_epi16(x, 4));
    x = _mm_or_si128(x, _mm_srli_epi16(x, 8));
    return x;
}

// Returns the max value for each 16-bit values.
inline int mm_hmax_epu16(__m128i x)
{
    // Unfortunately, there is no _mm_maxpos_epu16 builtin API.
    // Instead, use _mm_minpos_epu16 with negating the bits.
    __m128i not_maxpos = _mm_minpos_epu16(mm_not_si128(x));
    return (~_mm_cvtsi128_si32(not_maxpos)) & 0xFFFF;
}

// popcount 8 x 16bits
inline __m128i mm_popcnt_epi16(__m128i x)
{
    const __m128i mask4 = _mm_set1_epi8(0x0F);
    const __m128i lookup = _mm_setr_epi8(0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4);

    __m128i low = _mm_and_si128(mask4, x);
    __m128i high = _mm_and_si128(mask4, _mm_srli_epi16(x, 4));

    __m128i lowCount = _mm_shuffle_epi8(lookup, low);
    __m128i highCount = _mm_shuffle_epi8(lookup, high);
    __m128i count8 = _mm_add_epi8(lowCount, highCount);

    __m128i count16 = _mm_add_epi8(count8, _mm_slli_epi16(count8, 8));
    return _mm_srli_epi16(count16, 8);
}

}

#endif // BASE_SSE_H_
