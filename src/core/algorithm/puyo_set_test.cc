#include "core/algorithm/puyo_set.h"

#include <gtest/gtest.h>

TEST(PuyoSetTest, AddPuyoColor)
{
    PuyoSet set;
    set.add(PuyoColor::RED, 1);

    EXPECT_EQ(set.red(), 1U);
    EXPECT_EQ(set.blue(), 0U);
    EXPECT_EQ(set.yellow(), 0U);
    EXPECT_EQ(set.green(), 0U);
}

TEST(PuyoSetTest, AddPuyoSet)
{
    PuyoSet set1, set2;
    set1.add(PuyoColor::RED, 1);
    set1.add(PuyoColor::GREEN, 1);
    set2.add(PuyoColor::RED, 1);
    set2.add(PuyoColor::BLUE, 1);

    PuyoSet set;
    set.add(set1);
    set.add(set2);
    EXPECT_EQ(set.red(), 2U);
    EXPECT_EQ(set.blue(), 1U);
    EXPECT_EQ(set.yellow(), 0U);
    EXPECT_EQ(set.green(), 1U);
}
