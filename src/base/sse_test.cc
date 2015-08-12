#include "base/sse.h"

#include <gtest/gtest.h>

TEST(SSETest, inverseMovemask)
{
    for (int i = 0; i < 0x10000; ++i) {
        __m128i m = sse::inverseMovemask(i);
        int expected = _mm_movemask_epi8(m);

        EXPECT_EQ(expected, i);
    }
}

TEST(SSETest, mm_popcnt_epi16)
{
    union X {
        std::uint16_t v[8];
        __m128i m;
    };

    X m1 = { { 0x0000, 0x0001, 0x0010, 0x0100, 0x1000, 0x1100, 0x0011, 0x0101 } };
    X m2 = { { 0x1110, 0x1101, 0x1011, 0x0111, 0xFF00, 0x00FF, 0x0F0F, 0xFFFF } };

    X result;
    result.m = sse::mm_popcnt_epi16(m1.m);
    EXPECT_EQ(0, result.v[0]);
    EXPECT_EQ(1, result.v[1]);
    EXPECT_EQ(1, result.v[2]);
    EXPECT_EQ(1, result.v[3]);
    EXPECT_EQ(1, result.v[4]);
    EXPECT_EQ(2, result.v[5]);
    EXPECT_EQ(2, result.v[6]);
    EXPECT_EQ(2, result.v[7]);

    result.m = sse::mm_popcnt_epi16(m2.m);
    EXPECT_EQ(3, result.v[0]);
    EXPECT_EQ(3, result.v[1]);
    EXPECT_EQ(3, result.v[2]);
    EXPECT_EQ(3, result.v[3]);
    EXPECT_EQ(8, result.v[4]);
    EXPECT_EQ(8, result.v[5]);
    EXPECT_EQ(8, result.v[6]);
    EXPECT_EQ(16, result.v[7]);
}
