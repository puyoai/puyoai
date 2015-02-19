#include "pattern_book.h"

#include <gtest/gtest.h>

TEST(PatternBookField, complement1)
{
    PatternBookField pbf(
        "..BC.."
        "AAAB.."
        "BBBCCC");

    CoreField cf(
        "......"
        "..YB.."
        "BBBG..");

    CoreField expected(
        "..BG.."
        "YYYB.."
        "BBBGGG");

    ColumnPuyoList cpl;
    EXPECT_TRUE(pbf.complement(cf, &cpl));
    for (const auto& cp : cpl) {
        cf.dropPuyoOn(cp.x, cp.color);
    }
    EXPECT_EQ(expected, cf) << cf.toDebugString();
}

TEST(PatternBookField, complement2)
{
    PatternBookField pbf(
        "B....."
        "AAA..."
        "BC...."
        "BBC..."
        "CC....");

    CoreField cf(
        "YB...."
        "YYB..."
        "BBGGG.");

    // Since we cannot complement (3, 3), so this pattern should not match.
    ColumnPuyoList cpl;
    EXPECT_FALSE(pbf.complement(cf, &cpl));
}

TEST(PatternBookField, complement3)
{
    PatternBookField pbf(
        "BA...."
        "AA...."
        "BC...."
        "BBC..."
        "CC....");

    CoreField cf(
        " R    "
        "YY BBB"
        "RRRGGG");

    ColumnPuyoList cpl;
    EXPECT_FALSE(pbf.complement(cf, &cpl));
}

TEST(PatternBookField, complement4)
{
    PatternBookField pbf(
        "....De"
        "ABCDDE"
        "AABCCD"
        "BBCEEE");

    CoreField cf(
        ".....Y"
        "Y....Y"
        "Y..RRB"
        "GG.YYY");

    ColumnPuyoList cpl;
    EXPECT_TRUE(pbf.complement(cf, &cpl));
}

TEST(PatternBookField, ignoreable)
{
    PatternBookField pbf(
        ".BCD.."
        ".ABCD."
        ".ABCD."
        ".ABCD.");

    CoreField cf(
        ".G...."
        ".RG..."
        ".RGB.."
        ".RGB..");

    ColumnPuyoList cpl;
    EXPECT_FALSE(pbf.complement(cf, &cpl));

    cpl.clear();
    pbf.setIgnoreable('D');
    EXPECT_TRUE(pbf.complement(cf, &cpl));
}
