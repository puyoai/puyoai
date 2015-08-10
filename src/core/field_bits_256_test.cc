#ifdef __AVX2__

#include "core/field_bits_256.h"

#include <gtest/gtest.h>

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

#endif // __AVX2__
