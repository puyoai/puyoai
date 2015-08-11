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

    X m = { { 0x00, 0x10, 0x01, 0x33, 0x77, 0xF0, 0x0F, 0xFF } };

    X result;
    result.m = sse::mm_popcnt_epi16(m.m);
    EXPECT_EQ(0, result.v[0]);
    EXPECT_EQ(1, result.v[1]);
    EXPECT_EQ(1, result.v[2]);
    EXPECT_EQ(4, result.v[3]);
    EXPECT_EQ(6, result.v[4]);
    EXPECT_EQ(4, result.v[5]);
    EXPECT_EQ(4, result.v[6]);
    EXPECT_EQ(8, result.v[7]);
}
