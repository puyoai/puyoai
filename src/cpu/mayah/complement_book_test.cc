#include "complement_book.h"

#include <gtest/gtest.h>

TEST(ComplementBookFieldTest, complement1)
{
    ComplementBookField cbf(
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
    EXPECT_TRUE(cbf.complement(cf, &cpl));
    for (const auto& cp : cpl) {
        cf.dropPuyoOn(cp.x, cp.color);
    }
    EXPECT_EQ(expected, cf) << cf.toDebugString();
}

TEST(ComplementBookFieldTest, complement2)
{
    ComplementBookField cbf(
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
    EXPECT_FALSE(cbf.complement(cf, &cpl));
}

TEST(ComplementBookFieldTest, complement3)
{
    ComplementBookField cbf(
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
    EXPECT_FALSE(cbf.complement(cf, &cpl));
}

TEST(ComplementBookFieldTest, complement4)
{
    ComplementBookField cbf(
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
    EXPECT_TRUE(cbf.complement(cf, &cpl));
}

TEST(ComplementBookFieldTest, ignoreable)
{
    ComplementBookField cbf(
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
    EXPECT_FALSE(cbf.complement(cf, &cpl));

    cpl.clear();
    cbf.setIgnoreable('D');
    EXPECT_TRUE(cbf.complement(cf, &cpl));
}
