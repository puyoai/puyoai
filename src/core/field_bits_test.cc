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

TEST(FieldBitsTest, ctor2)
{
    PlainField pf("YB...."
                  "RYY..G"
                  "GRRYGG");
    FieldBits bits(pf, PuyoColor::RED);

    EXPECT_TRUE(bits.get(1, 2));
    EXPECT_TRUE(bits.get(2, 1));
    EXPECT_TRUE(bits.get(3, 1));

    EXPECT_FALSE(bits.get(1, 1));
    EXPECT_FALSE(bits.get(1, 3));
    EXPECT_FALSE(bits.get(2, 3));
    EXPECT_FALSE(bits.get(4, 2));
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

TEST(FieldBitsTest, popcount)
{
    FieldBits bits;
    bits.set(1, 1);
    bits.set(1, 2);
    bits.set(1, 3);
    bits.set(2, 4);
    bits.set(3, 1);
    bits.set(3, 2);
    bits.set(3, 3);
    bits.set(4, 3);
    bits.set(6, 9);

    EXPECT_EQ(9, bits.popcount());
}

TEST(FieldBitsTest, expand)
{
    FieldBits bits;
    bits.set(1, 1);
    bits.set(1, 2);
    bits.set(1, 3);
    bits.set(2, 4);
    bits.set(3, 1);
    bits.set(3, 2);
    bits.set(3, 3);
    bits.set(4, 3);

    FieldBits connected = bits.expand(3, 1);

    EXPECT_TRUE(connected.get(3, 1));
    EXPECT_TRUE(connected.get(3, 2));
    EXPECT_TRUE(connected.get(3, 3));
    EXPECT_TRUE(connected.get(4, 3));

    EXPECT_FALSE(connected.get(1, 1));
    EXPECT_FALSE(connected.get(1, 2));
    EXPECT_FALSE(connected.get(1, 3));
    EXPECT_FALSE(connected.get(1, 4));
    EXPECT_FALSE(connected.get(2, 1));
    EXPECT_FALSE(connected.get(2, 2));
    EXPECT_FALSE(connected.get(2, 3));
    EXPECT_FALSE(connected.get(2, 4));
    EXPECT_FALSE(connected.get(4, 1));
    EXPECT_FALSE(connected.get(4, 2));
}

TEST(FieldBitsTest, expand_exhaustive)
{
    // I O S Z L J T
    PlainField fi(
        "R....."
        "R....."
        "R....."
        "R....."
        "......"
        "RRRR..");
    PlainField fo(
        "RR...."
        "RR....");
    PlainField fs(
        "....R."
        ".RR.RR"
        "RR...R");
    PlainField fz(
        ".....R"
        "RR..RR"
        ".RR.R.");
    PlainField fl(
        "RR...."
        ".R...R"
        ".R.RRR"
        "R....."
        "R..RRR"
        "RR.R..");
    PlainField fj(
        "RR...."
        "R..R.."
        "R..RRR"
        ".R...."
        ".R.RRR"
        "RR...R");
    PlainField ft(
        ".R...."
        "RR..R."
        ".R.RRR"
        "R....."
        "RR.RRR"
        "R...R.");

    FieldBits bits[] {
        FieldBits(fi, PuyoColor::RED),
        FieldBits(fo, PuyoColor::RED),
        FieldBits(fs, PuyoColor::RED),
        FieldBits(fz, PuyoColor::RED),
        FieldBits(fl, PuyoColor::RED),
        FieldBits(fj, PuyoColor::RED),
        FieldBits(ft, PuyoColor::RED),
    };

    for (const FieldBits& fb : bits) {
        for (int x = 1; x <= 6; ++x) {
            for (int y = 1; y <= 12; ++y) {
                if (!fb.get(x, y))
                    continue;
                EXPECT_EQ(4, fb.expand(x, y).popcount());
                EXPECT_EQ(4, fb.expand4(x, y).popcount());
            }
        }
    }
}
