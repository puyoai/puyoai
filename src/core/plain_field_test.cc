#include "core/plain_field.h"

#include <gtest/gtest.h>

using namespace std;

TEST(PlainFieldTest, color)
{
    PlainField pf(
        "RGYB@&");

    EXPECT_EQ(PuyoColor::RED, pf.color(1, 1));
    EXPECT_EQ(PuyoColor::GREEN, pf.color(2, 1));
    EXPECT_EQ(PuyoColor::YELLOW, pf.color(3, 1));
    EXPECT_EQ(PuyoColor::BLUE, pf.color(4, 1));
    EXPECT_EQ(PuyoColor::OJAMA, pf.color(5, 1));
    EXPECT_EQ(PuyoColor::IRON, pf.color(6, 1));
    EXPECT_EQ(PuyoColor::EMPTY, pf.color(1, 2));
}

TEST(PlainFieldTest, isColor)
{
    PlainField pf(
        "RGYB@&");

    EXPECT_TRUE(pf.isColor(1, 1, PuyoColor::RED));
    EXPECT_TRUE(pf.isColor(2, 1, PuyoColor::GREEN));
    EXPECT_TRUE(pf.isColor(3, 1, PuyoColor::YELLOW));
    EXPECT_TRUE(pf.isColor(4, 1, PuyoColor::BLUE));
    EXPECT_TRUE(pf.isColor(5, 1, PuyoColor::OJAMA));
    EXPECT_TRUE(pf.isColor(6, 1, PuyoColor::IRON));
    EXPECT_TRUE(pf.isColor(1, 2, PuyoColor::EMPTY));
}

TEST(PlainFieldTest, isEmpty)
{
    PlainField pf(
        "RGYB@&");

    EXPECT_TRUE(pf.isEmpty(1, 2));
    EXPECT_FALSE(pf.isEmpty(1, 1));
}

TEST(PlainFieldTest, drop)
{
    PlainField pf(
        "RRRBBB"
        "......"
        "RRRBBB"
        "......"
        "RRRBBB");

    PlainField expected(
        "RRRBBB"
        "RRRBBB"
        "RRRBBB");

    pf.drop();
    EXPECT_TRUE(pf == expected);
}

TEST(PlainFieldTest, countConnectedPuyos)
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

    PlainField fields[] { fi, fo, fs, fz, fl, fj, ft };
    for (const PlainField& f : fields) {
        for (int x = 1; x <= 6; ++x) {
            for (int y = 1; y <= 12; ++y) {
                if (!f.isColor(x, y, PuyoColor::RED))
                    continue;
                EXPECT_EQ(4, f.countConnectedPuyosMax4(x, y));
                EXPECT_EQ(4, f.countConnectedPuyos(x, y));
            }
        }
    }
}

TEST(PlainFieldTest, countConnectedPuyosMax4EdgeCase)
{
    PlainField f(
      "YYYGGG" // 13
      "YYYGGG" // 12
      "OOOOOO"
      "OOOOOO"
      "OOOOOO"
      "OOOOOO" // 8
      "OOOOOO"
      "OOOOOO"
      "OOOOOO"
      "OOOOOO" // 4
      "OOOOOO"
      "OOOOOO"
      "OOOOOO");

    EXPECT_EQ(3, f.countConnectedPuyosMax4(1, 12));
    EXPECT_EQ(3, f.countConnectedPuyosMax4(2, 12));
    EXPECT_EQ(3, f.countConnectedPuyosMax4(3, 12));
    EXPECT_EQ(3, f.countConnectedPuyosMax4(4, 12));
    EXPECT_EQ(3, f.countConnectedPuyosMax4(5, 12));
    EXPECT_EQ(3, f.countConnectedPuyosMax4(6, 12));
}
