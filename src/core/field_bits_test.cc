#include "core/field_bits.h"

#include <algorithm>

#include "core/position.h"

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

TEST(FieldBitsTest, isEmpty)
{
    FieldBits bits;
    EXPECT_TRUE(bits.isEmpty());

    bits.set(1, 1);
    EXPECT_FALSE(bits.isEmpty());
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

TEST(FieldBitsTest, operator_equal)
{
    FieldBits bf1;
    bf1.set(1, 1);
    FieldBits bf2;
    bf2.set(1, 1);

    EXPECT_EQ(bf1, bf2);

    bf1.set(1, 2);

    EXPECT_NE(bf1, bf2);
}

TEST(FieldBitsTest, expand)
{
    FieldBits mask;
    mask.set(1, 1);
    mask.set(1, 2);
    mask.set(1, 3);
    mask.set(2, 4);
    mask.set(3, 1);
    mask.set(3, 2);
    mask.set(3, 3);
    mask.set(4, 3);

    FieldBits connected = FieldBits(3, 1).expand(mask);

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

TEST(FieldBitsTest, expand1)
{
    // bit     mask
    // ......  ..o...
    // ..xx..  .o..oo
    // ......  ..o...

    FieldBits mask;
    mask.set(2, 2);
    mask.set(3, 1);
    mask.set(3, 3);
    mask.set(5, 2);
    mask.set(6, 2);

    FieldBits bits;
    bits.set(3, 2);
    bits.set(4, 2);

    FieldBits expanded = bits.expand1(mask);

    EXPECT_TRUE(expanded.get(2, 2));
    EXPECT_TRUE(expanded.get(3, 1));
    EXPECT_TRUE(expanded.get(3, 3));
    EXPECT_TRUE(expanded.get(5, 2));

    EXPECT_FALSE(expanded.get(3, 2));
    EXPECT_FALSE(expanded.get(4, 2));
    EXPECT_FALSE(expanded.get(4, 3));
    EXPECT_FALSE(expanded.get(4, 1));

    EXPECT_FALSE(expanded.get(6, 2));
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
                EXPECT_EQ(4, FieldBits(x, y).expand(fb).popcount());
                EXPECT_EQ(4, FieldBits(x, y).expand4(fb).popcount());
            }
        }
    }
}

TEST(FieldBitsTest, vanishingSeed1)
{
    PlainField f(
        ".R...."
        "RR..R."
        ".R.RRR"
        "R...R."
        "RR.RRR"
        "R...R.");
    FieldBits fr(f, PuyoColor::RED);

    FieldBits seed = fr.vanishingSeed();
    FieldBits expanded = seed.expand(fr);

    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            if (f.color(x, y) == PuyoColor::RED)
                EXPECT_TRUE(expanded.get(x, y));
            else
                EXPECT_FALSE(expanded.get(x, y));
        }
    }
}

TEST(FieldBitsTest, vanishingSeed2)
{
    PlainField f(
        ".RRR.."
        "......"
        ".R.RR.");
    FieldBits fr(f, PuyoColor::RED);

    FieldBits seed = fr.vanishingSeed();
    FieldBits expanded = seed.expand(fr);

    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            EXPECT_FALSE(expanded.get(x, y));
        }
    }
}

TEST(FieldBitsTest, iterate)
{
    FieldBits bits;
    bits.set(1, 2);
    bits.set(2, 3);
    bits.set(3, 4);
    bits.set(4, 5);
    bits.set(5, 6);
    bits.set(6, 7);

    int count = 0;
    FieldBits iterated;
    bits.iterateBit([&iterated, &count](FieldBits x) {
        iterated.setAll(x);
        EXPECT_EQ(1, x.popcount());
        ++count;
    });

    EXPECT_EQ(bits, iterated);
    EXPECT_EQ(6, count);
}

TEST(FieldBitsTest, horizontalOr16)
{
    FieldBits bits;
    bits.set(1, 1);
    bits.set(2, 1);
    bits.set(3, 3);
    bits.set(3, 5);
    bits.set(4, 12);

    int x = bits.horizontalOr16();
    EXPECT_EQ((1 << 1) | (1 << 3) | (1 << 5) | (1 << 12), x);
}

TEST(FieldBitsTest, toPositions)
{
    FieldBits bits;
    bits.set(1, 1);
    bits.set(2, 2);
    bits.set(3, 3);
    bits.set(4, 4);
    bits.set(5, 5);
    bits.set(6, 6);

    Position ps[128];
    int len = bits.toPositions(ps);

    EXPECT_EQ(6, len);
    std::sort(ps, ps + len);

    EXPECT_EQ(Position(1, 1), ps[0]);
    EXPECT_EQ(Position(2, 2), ps[1]);
    EXPECT_EQ(Position(3, 3), ps[2]);
    EXPECT_EQ(Position(4, 4), ps[3]);
    EXPECT_EQ(Position(5, 5), ps[4]);
    EXPECT_EQ(Position(6, 6), ps[5]);
}
