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

TEST(PlainFieldTest, toFieldBits)
{
    PlainField pf("YB...."
                  "RYY..G"
                  "GRRYGG");
    FieldBits bits = pf.toFieldBits(PuyoColor::RED);

    EXPECT_TRUE(bits.get(1, 2));
    EXPECT_TRUE(bits.get(2, 1));
    EXPECT_TRUE(bits.get(3, 1));

    EXPECT_FALSE(bits.get(1, 1));
    EXPECT_FALSE(bits.get(1, 3));
    EXPECT_FALSE(bits.get(2, 3));
    EXPECT_FALSE(bits.get(4, 2));
}
