#include <immintrin.h>
#include <iostream>

#include <gtest/gtest.h>

#include "base/base.h"
#include "base/time_stamp_counter.h"
#include "core/field_bits.h"

using namespace std;

// popcount 8 x 16bits
NOINLINE_UNLESS_RELEASE __m128i f1(__m128i x)
{
    const __m128i mask4 = _mm_set1_epi8(0x0F);
    const __m128i lookup = _mm_setr_epi8(0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4);

    __m128i low = _mm_and_si128(mask4, x);
    __m128i high = _mm_and_si128(mask4, _mm_srli_epi16(x, 4));

    __m128i lowCount = _mm_shuffle_epi8(lookup, low);
    __m128i highCount = _mm_shuffle_epi8(lookup, high);
    __m128i count8 = _mm_add_epi8(lowCount, highCount);

    __m128i count16 = _mm_add_epi8(count8, _mm_srli_epi16(count8, 8));
    __m128i count = _mm_and_si128(count16, _mm_set1_epi16(0xFF));

    return count;
}

// This is slower than the previous one.
__m128i f2(__m128i x)
{
    const __m128i mask1 = _mm_set1_epi8(0x55);
    __m128i v1_low = _mm_and_si128(mask1, x);
    __m128i v1_high = _mm_and_si128(mask1, _mm_srli_epi16(x, 1));
    __m128i v1 = _mm_add_epi8(v1_low, v1_high);

    const __m128i mask2 = _mm_set1_epi8(0x33);
    __m128i v2_low = _mm_and_si128(mask2, v1);
    __m128i v2_high = _mm_and_si128(mask2, _mm_srli_epi16(v1, 2));
    __m128i v2 = _mm_add_epi8(v2_low, v2_high);

    const __m128i mask3 = _mm_set1_epi8(0x0F);
    __m128i v3_low = _mm_and_si128(mask3, v2);
    __m128i v3_high = _mm_and_si128(mask3, _mm_srli_epi16(v2, 4));
    __m128i v3 = _mm_add_epi8(v3_low, v3_high);

    __m128i count16 = _mm_add_epi8(v3, _mm_srli_epi16(v3, 8));
    return _mm_and_si128(count16, _mm_set1_epi16(0xFF));
}

TEST(Popcount, experimental)
{
    const int N = 10000;
    const FieldBits x = _mm_set_epi16(0x0, 0x1101, 0x1011, 0x0111, 0xFF00, 0x00FF, 0x0F0F, 0xFFFF);
    const FieldBits expected = _mm_set_epi16(0, 3, 3, 3, 8, 8, 8, 16);

    TimeStampCounterData d1;
    TimeStampCounterData d2;

    for (int i = 0; i < N; ++i) {
        ScopedTimeStampCounter stsc(&d1);
        EXPECT_EQ(expected, FieldBits(f1(x)));
    }
    for (int i = 0; i < N; ++i) {
        ScopedTimeStampCounter stsc(&d2);
        EXPECT_EQ(expected, FieldBits(f2(x)));
    }

    cout << "f1" << endl;
    d1.showStatistics();
    cout << "f2" << endl;
    d2.showStatistics();
}
