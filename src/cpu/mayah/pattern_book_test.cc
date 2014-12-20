#include "pattern_book.h"

#include <gtest/gtest.h>

TEST(PatternBookField, complement1)
{
    PatternBookField pbf(
        ".*23.."
        "1112.."
        "222333");

    CoreField cf(
        "......"
        "..YB.."
        "BBBG..");

    CoreField expected(
        ".YBG.."
        "YYYB.."
        "BBBGGG");

    ColumnPuyoList cpl;
    EXPECT_TRUE(pbf.complement(cf, &cpl));
    for (const auto& cp : cpl) {
        cf.dropPuyoOn(cp.x, cp.color);
    }
    EXPECT_EQ(expected, cf);
}

TEST(PatternBookField, complement2)
{
    PatternBookField pbf(
        "2....."
        "111..."
        "23...."
        "223..."
        "33....");

    CoreField cf(
        "YB...."
        "YYB..."
        "BBGGG.");

    // Since we cannot complement (3, 3), so this pattern should not match.
    ColumnPuyoList cpl;
    EXPECT_FALSE(pbf.complement(cf, &cpl));
}
