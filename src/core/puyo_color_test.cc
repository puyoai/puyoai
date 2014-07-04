#include "core/puyo_color.h"

#include <gtest/gtest.h>

TEST(PuyoColorTest, isColorPuyo)
{
    EXPECT_FALSE(isNormalColor(PuyoColor::EMPTY));
    EXPECT_FALSE(isNormalColor(PuyoColor::OJAMA));
    EXPECT_FALSE(isNormalColor(PuyoColor::WALL));

    EXPECT_TRUE(isNormalColor(PuyoColor::RED));
    EXPECT_TRUE(isNormalColor(PuyoColor::BLUE));
    EXPECT_TRUE(isNormalColor(PuyoColor::YELLOW));
    EXPECT_TRUE(isNormalColor(PuyoColor::GREEN));
}

TEST(PuyoColorTest, toChar)
{
    EXPECT_EQ(toChar(PuyoColor::EMPTY), ' ');
    EXPECT_EQ(toChar(PuyoColor::OJAMA), '@');
    EXPECT_EQ(toChar(PuyoColor::WALL), '#');
    EXPECT_EQ(toChar(PuyoColor::RED), 'R');
    EXPECT_EQ(toChar(PuyoColor::BLUE), 'B');
    EXPECT_EQ(toChar(PuyoColor::YELLOW), 'Y');
    EXPECT_EQ(toChar(PuyoColor::GREEN), 'G');
}

TEST(PuyoColorTest, convertible)
{
    EXPECT_EQ(PuyoColor::EMPTY,  toPuyoColor(toChar(PuyoColor::EMPTY)));
    EXPECT_EQ(PuyoColor::OJAMA,  toPuyoColor(toChar(PuyoColor::OJAMA)));
    EXPECT_EQ(PuyoColor::WALL,   toPuyoColor(toChar(PuyoColor::WALL)));
    EXPECT_EQ(PuyoColor::RED,    toPuyoColor(toChar(PuyoColor::RED)));
    EXPECT_EQ(PuyoColor::BLUE,   toPuyoColor(toChar(PuyoColor::BLUE)));
    EXPECT_EQ(PuyoColor::YELLOW, toPuyoColor(toChar(PuyoColor::YELLOW)));
    EXPECT_EQ(PuyoColor::GREEN,  toPuyoColor(toChar(PuyoColor::GREEN)));
}
