#include "core/algorithm/puyo_set.h"

#include <gtest/gtest.h>

TEST(PuyoSetTest, addPuyoColor)
{
    PuyoSet set;
    set.add(PuyoColor::RED, 1);

    EXPECT_EQ(1, set.red());
    EXPECT_EQ(0, set.blue());
    EXPECT_EQ(0, set.yellow());
    EXPECT_EQ(0, set.green());
}

TEST(PuyoSetTest, addPuyoSet)
{
    PuyoSet set1, set2;
    set1.add(PuyoColor::RED, 1);
    set1.add(PuyoColor::GREEN, 1);
    set2.add(PuyoColor::RED, 1);
    set2.add(PuyoColor::BLUE, 1);

    PuyoSet set;
    set.add(set1);
    set.add(set2);
    EXPECT_EQ(2, set.red());
    EXPECT_EQ(1, set.blue());
    EXPECT_EQ(0, set.yellow());
    EXPECT_EQ(1, set.green());
}
