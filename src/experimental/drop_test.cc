#include <immintrin.h>
#include <iostream>
#include <iomanip>

#include <gtest/gtest.h>

#include "base/base.h"
#include "base/builtin.h"
#include "base/avx.h"
#include "base/sse.h"
#include "base/time.h"
#include "base/time_stamp_counter.h"
#include "core/field_bits.h"

using namespace std;

#ifdef __BMI2__
void f1(FieldBits m_[3], FieldBits erased)
{
    const FieldBits fieldMask = FieldBits::FIELD_MASK_13;
    const FieldBits leftBits = fieldMask.notmask(erased);
    sse::Decomposer x;
    x.m = leftBits;
    const std::uint64_t oldLowBits = x.ui64[0];
    const std::uint64_t oldHighBits = x.ui64[1];

    sse::Decomposer height;
    height.m = sse::mm_popcnt_epi16(leftBits);

    const std::uint64_t newLowBits =
        (((1ULL << height.ui16[1]) - 1) << 17) |
        (((1ULL << height.ui16[2]) - 1) << 33) |
        (((1ULL << height.ui16[3]) - 1) << 49);
    const std::uint64_t newHighBits =
        (((1ULL << height.ui16[4]) - 1) << 1) |
        (((1ULL << height.ui16[5]) - 1) << 17) |
        (((1ULL << height.ui16[6]) - 1) << 33);

    for (int i = 0; i < 3; ++i) {
        sse::Decomposer d;
        d.m = m_[i];

        std::uint64_t extLow = _pext_u64(d.ui64[0], oldLowBits);
        std::uint64_t depLow = _pdep_u64(extLow, newLowBits);

        std::uint64_t extHigh = _pext_u64(d.ui64[1], oldHighBits);
        std::uint64_t depHigh = _pdep_u64(extHigh, newHighBits);

        d.ui64[0] = depLow;
        d.ui64[1] = depHigh;
        m_[i] = d.m;
    }
}

void f2(FieldBits m_[3], FieldBits erased)
{
    const FieldBits fieldMask = FieldBits::FIELD_MASK_13;
    const FieldBits leftBits = fieldMask.notmask(erased);

    int column1 = _mm_extract_epi16(leftBits, 1);
    for (int i = 0; i < 3; ++i) {
        int v = _pext_u64(_mm_extract_epi16(m_[i], 1), column1) << 1;
        m_[i] = _mm_insert_epi16(m_[i], v, 1);
    }

    int column2 = _mm_extract_epi16(leftBits, 2);
    for (int i = 0; i < 3; ++i) {
        int v = _pext_u64(_mm_extract_epi16(m_[i], 2), column2) << 1;
        m_[i] = _mm_insert_epi16(m_[i], v, 2);
    }

    int column3 = _mm_extract_epi16(leftBits, 3);
    for (int i = 0; i < 3; ++i) {
        int v = _pext_u64(_mm_extract_epi16(m_[i], 3), column3) << 1;
        m_[i] = _mm_insert_epi16(m_[i], v, 3);
    }

    int column4 = _mm_extract_epi16(leftBits, 4);
    for (int i = 0; i < 3; ++i) {
        int v = _pext_u64(_mm_extract_epi16(m_[i], 4), column4) << 1;
        m_[i] = _mm_insert_epi16(m_[i], v, 4);
    }

    int column5 = _mm_extract_epi16(leftBits, 5);
    for (int i = 0; i < 3; ++i) {
        int v = _pext_u64(_mm_extract_epi16(m_[i], 5), column5) << 1;
        m_[i] = _mm_insert_epi16(m_[i], v, 5);
    }

    int column6 = _mm_extract_epi16(leftBits, 6);
    for (int i = 0; i < 3; ++i) {
        int v = _pext_u64(_mm_extract_epi16(m_[i], 6), column6) << 1;
        m_[i] = _mm_insert_epi16(m_[i], v, 6);
    }
}

void f3(FieldBits m_[3], FieldBits erased)
{
    const FieldBits fieldMask = FieldBits::FIELD_MASK_13;
    const FieldBits leftBits = fieldMask.notmask(erased);

    int column1 = _mm_extract_epi16(leftBits, 1);
    int column2 = _mm_extract_epi16(leftBits, 2);
    int column3 = _mm_extract_epi16(leftBits, 3);
    int column4 = _mm_extract_epi16(leftBits, 4);
    int column5 = _mm_extract_epi16(leftBits, 5);
    int column6 = _mm_extract_epi16(leftBits, 6);

    uint16_t vs[8] {};

    for (int i = 0; i < 3; ++i) {
        vs[1] = _pext_u64(_mm_extract_epi16(m_[i], 1), column1) << 1;
        vs[2] = _pext_u64(_mm_extract_epi16(m_[i], 2), column2) << 1;
        vs[3] = _pext_u64(_mm_extract_epi16(m_[i], 3), column3) << 1;
        vs[4] = _pext_u64(_mm_extract_epi16(m_[i], 4), column4) << 1;
        vs[5] = _pext_u64(_mm_extract_epi16(m_[i], 5), column5) << 1;
        vs[6] = _pext_u64(_mm_extract_epi16(m_[i], 6), column6) << 1;

        m_[i] = _mm_set_epi16(0, vs[6], vs[5], vs[4], vs[3], vs[2], vs[1], 0);
    }
}

void f4(FieldBits m_[3], FieldBits erased)
{
    const FieldBits fieldMask = FieldBits::FIELD_MASK_13;
    const FieldBits leftBits = fieldMask.notmask(erased);

    int column1 = _mm_extract_epi16(leftBits, 1);
    int column2 = _mm_extract_epi16(leftBits, 2);
    int column3 = _mm_extract_epi16(leftBits, 3);
    int column4 = _mm_extract_epi16(leftBits, 4);
    int column5 = _mm_extract_epi16(leftBits, 5);
    int column6 = _mm_extract_epi16(leftBits, 6);

    uint16_t vs[8] {};

    for (int i = 0; i < 3; ++i) {
        vs[1] = _pext_u64(_mm_extract_epi16(m_[i], 1), column1);
        vs[2] = _pext_u64(_mm_extract_epi16(m_[i], 2), column2);
        vs[3] = _pext_u64(_mm_extract_epi16(m_[i], 3), column3);
        vs[4] = _pext_u64(_mm_extract_epi16(m_[i], 4), column4);
        vs[5] = _pext_u64(_mm_extract_epi16(m_[i], 5), column5);
        vs[6] = _pext_u64(_mm_extract_epi16(m_[i], 6), column6);

        m_[i] = _mm_set_epi16(0, vs[6], vs[5], vs[4], vs[3], vs[2], vs[1], 0);
        m_[i] = _mm_add_epi16(m_[i], m_[i]);
    }
}

void f5(FieldBits m_[3], FieldBits erased)
{
    const FieldBits fieldMask = FieldBits::FIELD_MASK_13;
    const FieldBits leftBits = fieldMask.notmask(erased);
    sse::Decomposer x;
    x.m = leftBits;
    const std::uint64_t oldLowBits = x.ui64[0];
    const std::uint64_t oldHighBits = x.ui64[1];

    const __m256i ones = _mm256_set_epi32(0, 1, 1, 1, 1, 1, 1, 0);
    __m256i height = _mm256_cvtepi16_epi32(sse::mm_popcnt_epi16(leftBits));
    height = _mm256_sllv_epi32(ones, height);
    height = _mm256_sub_epi32(height, ones);
    height = _mm256_slli_epi32(height, 1);

    height = _mm256_packs_epi32(height, height);
    avx::Decomposer256 y;
    y.m = height;
    const std::uint64_t newLowBits = y.ui64[0];
    const std::uint64_t newHighBits = y.ui64[2];

    for (int i = 0; i < 3; ++i) {
        sse::Decomposer d;
        d.m = m_[i];

        std::uint64_t extLow = _pext_u64(d.ui64[0], oldLowBits);
        std::uint64_t depLow = _pdep_u64(extLow, newLowBits);

        std::uint64_t extHigh = _pext_u64(d.ui64[1], oldHighBits);
        std::uint64_t depHigh = _pdep_u64(extHigh, newHighBits);

        d.ui64[0] = depLow;
        d.ui64[1] = depHigh;
        m_[i] = d.m;
    }
}

int f6(FieldBits m_[3], FieldBits erased)
{
    const FieldBits fieldMask = FieldBits::FIELD_MASK_13;
    const FieldBits leftBits = fieldMask.notmask(erased);

    sse::Decomposer t;

    int maxDrops = 0;
    t.m = (m_[0] | m_[1] | m_[2]).notmask(erased);
    for (int x = 1; x <= 6; ++x) {
        if (t.ui16[x] == 0)
            continue;
        int h = 31 - countLeadingZeros32(t.ui16[x]);
        int p = popCount32(t.ui16[x] ^ (((1 << h) - 1) << 1));
        cout << p << endl;
        maxDrops = std::max(p, maxDrops);
    }

    t.m = leftBits;
    const std::uint64_t oldLowBits = t.ui64[0];
    const std::uint64_t oldHighBits = t.ui64[1];

    const __m256i ones = _mm256_set_epi32(0, 1, 1, 1, 1, 1, 1, 0);
    __m256i height = _mm256_cvtepi16_epi32(sse::mm_popcnt_epi16(leftBits));
    height = _mm256_sllv_epi32(ones, height);
    height = _mm256_sub_epi32(height, ones);
    height = _mm256_slli_epi32(height, 1);

    height = _mm256_packs_epi32(height, height);
    avx::Decomposer256 y;
    y.m = height;
    const std::uint64_t newLowBits = y.ui64[0];
    const std::uint64_t newHighBits = y.ui64[2];

    for (int i = 0; i < 3; ++i) {
        sse::Decomposer d;
        d.m = m_[i];
        d.ui64[0] = _pdep_u64(_pext_u64(d.ui64[0], oldLowBits), newLowBits);
        d.ui64[1] = _pdep_u64(_pext_u64(d.ui64[1], oldHighBits), newHighBits);
        m_[i] = d.m;
    }

    return maxDrops;
}

TEST(DropTest, warmup)
{
    const int N = 10000000;

    const FieldBits fb(
        "111111"
        "111111");
    const FieldBits expected(
        ".....1"
        "1111.1");

    const FieldBits erased(
        "....1."
        "11111."
    );

    for (int i = 0; i < N; ++i) {
        FieldBits bits[3] = { fb, fb, fb };
        f1(bits, erased);
        EXPECT_EQ(expected, bits[0]);
    }
}

TEST(DropTest, test1)
{
    const int N = 10000000;

    const FieldBits fb(
        "111111"
        "111111");
    const FieldBits expected(
        ".....1"
        "1111.1");

    const FieldBits erased(
        "....1."
        "11111."
    );

    for (int i = 0; i < N; ++i) {
        FieldBits bits[3] = { fb, fb, fb };
        f1(bits, erased);
        EXPECT_EQ(expected, bits[0]);
    }
}

TEST(DropTest, test2)
{
    const int N = 10000000;

    const FieldBits fb(
        "111111"
        "111111");
    const FieldBits expected(
        ".....1"
        "1111.1");

    const FieldBits erased(
        "....1."
        "11111."
    );

    for (int i = 0; i < N; ++i) {
        FieldBits bits[3] = { fb, fb, fb };
        f2(bits, erased);
        EXPECT_EQ(expected, bits[0]);
    }
}

TEST(DropTest, test3)
{
    const int N = 10000000;

    const FieldBits fb(
        "111111"
        "111111");
    const FieldBits expected(
        ".....1"
        "1111.1");

    const FieldBits erased(
        "....1."
        "11111."
    );

    for (int i = 0; i < N; ++i) {
        FieldBits bits[3] = { fb, fb, fb };
        f3(bits, erased);
        EXPECT_EQ(expected, bits[0]);
    }
}

TEST(DropTest, test4)
{
    const int N = 10000000;

    const FieldBits fb(
        "111111"
        "111111");
    const FieldBits expected(
        ".....1"
        "1111.1");

    const FieldBits erased(
        "....1."
        "11111."
    );

    for (int i = 0; i < N; ++i) {
        FieldBits bits[3] = { fb, fb, fb };
        f4(bits, erased);
        EXPECT_EQ(expected, bits[0]);
    }
}

TEST(DropTest, test5)
{
    const int N = 10000000;

    const FieldBits fb(
        "111111"
        "111111");
    const FieldBits expected(
        ".....1"
        "1111.1");

    const FieldBits erased(
        "....1."
        "11111."
    );

    for (int i = 0; i < N; ++i) {
        FieldBits bits[3] = { fb, fb, fb };
        f5(bits, erased);
        EXPECT_EQ(expected, bits[0]);
    }
}

TEST(DropTest, test6)
{
    const int N = 1;

    const FieldBits fb(
        "111111"
        "111111"
        "111111");
    const FieldBits expected(
        "......"
        "....11"
        "11..11");
    const FieldBits erased(
        "111111"
        "..11.."
        "1111.."
    );

    for (int i = 0; i < N; ++i) {
        FieldBits bits[3] = { fb, fb, fb };
        f6(bits, erased);
        EXPECT_EQ(expected, bits[0]);
    }
}

#endif
