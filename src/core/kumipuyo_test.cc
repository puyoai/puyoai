#include "core/kumipuyo.h"

#include <gtest/gtest.h>

TEST(KumipuyoTest, ctor)
{
    Kumipuyo k;
    EXPECT_EQ(PuyoColor::EMPTY, k.axis);
    EXPECT_EQ(PuyoColor::EMPTY, k.child);

    k = Kumipuyo(PuyoColor::RED, PuyoColor::BLUE);
    EXPECT_EQ(PuyoColor::RED, k.axis);
    EXPECT_EQ(PuyoColor::BLUE, k.child);
}

TEST(KumipuyoTest, equal)
{
    EXPECT_EQ(Kumipuyo(PuyoColor::RED, PuyoColor::BLUE), Kumipuyo(PuyoColor::RED, PuyoColor::BLUE));
    EXPECT_NE(Kumipuyo(PuyoColor::RED, PuyoColor::BLUE), Kumipuyo(PuyoColor::RED, PuyoColor::RED));
    EXPECT_NE(Kumipuyo(PuyoColor::RED, PuyoColor::BLUE), Kumipuyo(PuyoColor::BLUE, PuyoColor::BLUE));
}
