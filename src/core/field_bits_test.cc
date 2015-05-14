#include "core/field_bits.h"

#include <gtest/gtest.h>

using namespace std;

TEST(FieldBitsTest, ctor)
{
    FieldBits bits;
    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 16; ++y) {
            EXPECT_FALSE(bits.get(x, y));
        }
    }
}

TEST(FieldBitsTest, set)
{
    FieldBits bits;
    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 16; ++y) {
            bits.set(x, y);
        }
    }

    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 16; ++y) {
            EXPECT_TRUE(bits.get(x, y));
        }
    }
}

TEST(FieldBitsTest, unset)
{
    FieldBits bits;
    bits.set(1, 1);
    bits.set(1, 2);
    bits.set(1, 3);

    EXPECT_TRUE(bits.get(1, 1));
    EXPECT_TRUE(bits.get(1, 2));
    EXPECT_TRUE(bits.get(1, 3));

    bits.unset(1, 1);
    bits.unset(1, 2);

    EXPECT_FALSE(bits.get(1, 1));
    EXPECT_FALSE(bits.get(1, 2));
    EXPECT_TRUE(bits.get(1, 3));

}
