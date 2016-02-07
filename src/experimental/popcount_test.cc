#include <immintrin.h>
#include <iostream>

#include <gtest/gtest.h>

#include "base/base.h"
#include "base/builtin.h"
#include "base/time.h"
#include "base/time_stamp_counter.h"
#include "core/field_bits.h"

using namespace std;

// popcount 8 x 16bits
__m128i f1(__m128i x)
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

__m128i f3(__m128i value)
{
    union {
        std::uint16_t v[8];
        __m128i m;
    };

    m = value;

    for (int x = 1; x <= 6; ++x) {
        v[x] = popCount32(v[x]);
    }

    return m;
}

__m128i f4(__m128i value)
{
    value = _mm_insert_epi16(value, popCount32(_mm_extract_epi16(value, 1)), 1);
    value = _mm_insert_epi16(value, popCount32(_mm_extract_epi16(value, 2)), 2);
    value = _mm_insert_epi16(value, popCount32(_mm_extract_epi16(value, 3)), 3);
    value = _mm_insert_epi16(value, popCount32(_mm_extract_epi16(value, 4)), 4);
    value = _mm_insert_epi16(value, popCount32(_mm_extract_epi16(value, 5)), 5);
    value = _mm_insert_epi16(value, popCount32(_mm_extract_epi16(value, 6)), 6);

    return value;
}

__m128i f5(__m128i value)
{
    int v1 = popCount32(_mm_extract_epi16(value, 1));
    int v2 = popCount32(_mm_extract_epi16(value, 2));
    int v3 = popCount32(_mm_extract_epi16(value, 3));
    int v4 = popCount32(_mm_extract_epi16(value, 4));
    int v5 = popCount32(_mm_extract_epi16(value, 5));
    int v6 = popCount32(_mm_extract_epi16(value, 6));

    return _mm_set_epi16(0, v6, v5, v4, v3, v2, v1, 0);
}

__m128i f6(__m128i value)
{
    int v1 = popCount32(_mm_extract_epi16(value, 1));
    int v2 = popCount32(_mm_extract_epi16(value, 2));
    int v3 = popCount32(_mm_extract_epi16(value, 3));
    int v4 = popCount32(_mm_extract_epi16(value, 4));
    int v5 = popCount32(_mm_extract_epi16(value, 5));
    int v6 = popCount32(_mm_extract_epi16(value, 6));

    value = _mm_insert_epi16(value, v1, 1);
    value = _mm_insert_epi16(value, v2, 2);
    value = _mm_insert_epi16(value, v3, 3);
    value = _mm_insert_epi16(value, v4, 4);
    value = _mm_insert_epi16(value, v5, 5);
    value = _mm_insert_epi16(value, v6, 6);
    return value;
}

TEST(Popcount, experimental_warmup)
{
    const int N = 100000;
    const FieldBits x = _mm_set_epi16(0x0, 0x1101, 0x1011, 0x0111, 0xFF00, 0x00FF, 0xFFFF, 0);
    const FieldBits expected = _mm_set_epi16(0, 3, 3, 3, 8, 8, 16, 0);

    TimeStampCounterData d;
    for (int i = 0; i < N; ++i) {
        ScopedTimeStampCounter stsc(&d);
        EXPECT_EQ(expected, FieldBits(f1(x)));
    }

    d.showStatistics();
}

TEST(Popcount, experimental_f1)
{
    const int N = 10000000;
    const FieldBits x = _mm_set_epi16(0x0, 0x1101, 0x1011, 0x0111, 0xFF00, 0x00FF, 0xFFFF, 0);
    const FieldBits expected = _mm_set_epi16(0, 3, 3, 3, 8, 8, 16, 0);

    TimeStampCounterData d;
    for (int i = 0; i < N; ++i) {
        ScopedTimeStampCounter stsc(&d);
        EXPECT_EQ(expected, FieldBits(f1(x)));
    }

    d.showStatistics();
}

TEST(Popcount, experimental_f2)
{
    const int N = 10000000;
    const FieldBits x = _mm_set_epi16(0x0, 0x1101, 0x1011, 0x0111, 0xFF00, 0x00FF, 0xFFFF, 0);
    const FieldBits expected = _mm_set_epi16(0, 3, 3, 3, 8, 8, 16, 0);

    TimeStampCounterData d;
    for (int i = 0; i < N; ++i) {
        ScopedTimeStampCounter stsc(&d);
        EXPECT_EQ(expected, FieldBits(f2(x)));
    }

    d.showStatistics();
}

TEST(Popcount, experimental_f3)
{
    const int N = 10000000;
    const FieldBits x = _mm_set_epi16(0x0, 0x1101, 0x1011, 0x0111, 0xFF00, 0x00FF, 0xFFFF, 0);
    const FieldBits expected = _mm_set_epi16(0, 3, 3, 3, 8, 8, 16, 0);

    TimeStampCounterData d;
    for (int i = 0; i < N; ++i) {
        ScopedTimeStampCounter stsc(&d);
        EXPECT_EQ(expected, FieldBits(f3(x)));
    }

    d.showStatistics();
}

TEST(Popcount, experimental_f4)
{
    const int N = 10000000;
    const FieldBits x = _mm_set_epi16(0x0, 0x1101, 0x1011, 0x0111, 0xFF00, 0x00FF, 0xFFFF, 0);
    const FieldBits expected = _mm_set_epi16(0, 3, 3, 3, 8, 8, 16, 0);

    TimeStampCounterData d;
    for (int i = 0; i < N; ++i) {
        ScopedTimeStampCounter stsc(&d);
        EXPECT_EQ(expected, FieldBits(f4(x)));
    }

    d.showStatistics();
}

TEST(Popcount, experimental_f5)
{
    const int N = 10000000;
    const FieldBits x = _mm_set_epi16(0x0, 0x1101, 0x1011, 0x0111, 0xFF00, 0x00FF, 0xFFFF, 0);
    const FieldBits expected = _mm_set_epi16(0, 3, 3, 3, 8, 8, 16, 0);

    TimeStampCounterData d;
    for (int i = 0; i < N; ++i) {
        ScopedTimeStampCounter stsc(&d);
        EXPECT_EQ(expected, FieldBits(f5(x)));
    }

    d.showStatistics();
}

TEST(Popcount, experimental_f6)
{
    const int N = 10000000;
    const FieldBits x = _mm_set_epi16(0x0, 0x1101, 0x1011, 0x0111, 0xFF00, 0x00FF, 0xFFFF, 0);
    const FieldBits expected = _mm_set_epi16(0, 3, 3, 3, 8, 8, 16, 0);

    TimeStampCounterData d;
    for (int i = 0; i < N; ++i) {
        ScopedTimeStampCounter stsc(&d);
        EXPECT_EQ(expected, FieldBits(f6(x)));
    }

    d.showStatistics();
}
