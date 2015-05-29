#include "base/bmi.h"

#include <gtest/gtest.h>

TEST(BMITest, pext)
{
    //      x: 10101010
    //   mask: 11000111
    // result: 00010010

    uint64_t x = 0xAA;
    uint64_t mask = 0xC7;
    EXPECT_EQ(0x12UL, bmi::extractBits(x, mask));
}

TEST(BMITest, pdep)
{
    //      x: 10101010
    //   mask: 11000111
    // result: 01000010

    uint64_t x = 0xAA;
    uint64_t mask = 0xC7;
    EXPECT_EQ(0x42UL, bmi::depositBits(x, mask));
}
