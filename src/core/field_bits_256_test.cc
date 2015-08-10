#ifdef __AVX2__

#include <gtest/gtest.h>

#include "core/bit_field.h"
#include "core/field_bits_256.h"

using namespace std;

TEST(FieldBits256Test, ctor1)
{
    FieldBits256 bits;
    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 16; ++y) {
            EXPECT_FALSE(bits.get(FieldBits256::HighLow::LOW, x, y));
            EXPECT_FALSE(bits.get(FieldBits256::HighLow::HIGH, x, y));
        }
    }
}

TEST(FieldBits256Test, ctor2)
{
    FieldBits low;
    low.set(1, 3);
    low.set(4, 8);
    FieldBits high;
    high.set(2, 4);
    high.set(5, 9);

    FieldBits256 fb256(high, low);

    EXPECT_TRUE(fb256.get(FieldBits256::HighLow::LOW, 1, 3));
    EXPECT_TRUE(fb256.get(FieldBits256::HighLow::LOW, 4, 8));
    EXPECT_TRUE(fb256.get(FieldBits256::HighLow::HIGH, 2, 4));
    EXPECT_TRUE(fb256.get(FieldBits256::HighLow::HIGH, 5, 9));

    EXPECT_FALSE(fb256.get(FieldBits256::HighLow::HIGH, 1, 3));
    EXPECT_FALSE(fb256.get(FieldBits256::HighLow::HIGH, 4, 8));
    EXPECT_FALSE(fb256.get(FieldBits256::HighLow::LOW, 2, 4));
    EXPECT_FALSE(fb256.get(FieldBits256::HighLow::LOW, 5, 9));
}

TEST(FieldBits256Test, expand)
{
    FieldBits maskHigh(
        "..1..."
        "..1.11"
        "111.11");
    FieldBits maskLow(
        "111111"
        ".....1"
        "111111"
        "1....."
        "111111");

    FieldBits256 mask(maskHigh, maskLow);

    FieldBits256 bit;
    bit.setHigh(3, 1);
    bit.setLow(6, 1);

    FieldBits256 expanded = bit.expand(mask);

    FieldBits highExpected = FieldBits(3, 1).expand(maskHigh);
    FieldBits lowExpected = FieldBits(6, 1).expand(maskLow);

    EXPECT_EQ(highExpected, expanded.high());
    EXPECT_EQ(lowExpected, expanded.low());
    EXPECT_EQ(maskLow, expanded.low());
}

TEST(FieldBits256Test, vanishingSeed)
{
    BitField bf(
        ".....R"
        ".RR..R"
        "YYRBBR"
        "RYYBBG"
        "RRRGGG");

    FieldBits red = bf.bits(PuyoColor::RED);
    FieldBits blue = bf.bits(PuyoColor::BLUE);
    FieldBits yellow = bf.bits(PuyoColor::YELLOW);
    FieldBits green = bf.bits(PuyoColor::GREEN);

    FieldBits256 redBlueSeed = FieldBits256(red, blue).vanishingSeed();
    FieldBits256 yellowGreenSeed = FieldBits256(yellow, green).vanishingSeed();

    EXPECT_EQ(FieldBits256(red.vanishingSeed(), blue.vanishingSeed()), redBlueSeed);
    EXPECT_EQ(FieldBits256(yellow.vanishingSeed(), green.vanishingSeed()), yellowGreenSeed);
}

#endif // __AVX2__
