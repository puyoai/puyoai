#ifndef BASE_SSE_H_
#define BASE_SSE_H_

#include <smmintrin.h>

namespace sse {

// Makes __m128i x s.t. mask = _mm_movemask_epi8(x).
inline
__m128i inverseMovemask(int mask)
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

}

#endif // BASE_SSE_H_
