#include <immintrin.h>
#include <iostream>
#include <iomanip>

#include <gtest/gtest.h>

#include "base/base.h"
#include "base/sse.h"
#include "base/time.h"
#include "base/time_stamp_counter.h"
#include "core/field_bits.h"

using namespace std;

#ifdef __BMI2__
void f1(FieldBits m_[3], FieldBits erased)
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

    const FieldBits fieldMask = FieldBits::FIELD_MASK_13;
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

#endif
