#include "core/column_puyo_list.h"

#include <gtest/gtest.h>

TEST(ColumnPuyoListTest, constructor)
{
    ColumnPuyoList cpl;
    EXPECT_EQ(0, cpl.size());
    EXPECT_TRUE(cpl.isEmpty());
}

TEST(ColumnPuyoListTest, add)
{
    ColumnPuyoList cpl;
    ASSERT_TRUE(cpl.add(3, PuyoColor::RED, 2));

    EXPECT_EQ(2, cpl.size());
    EXPECT_FALSE(cpl.isEmpty());
}

TEST(ColumnPuyoListTest, append)
{
    ColumnPuyoList cpl;
    ColumnPuyoList cpl1;
    ASSERT_TRUE(cpl1.add(3, PuyoColor::RED, 2));
    ColumnPuyoList cpl2;
    ASSERT_TRUE(cpl2.add(3, PuyoColor::BLUE, 2));
    ColumnPuyoList cpl3;
    ASSERT_TRUE(cpl3.add(3, PuyoColor::BLUE, 8));

    EXPECT_TRUE(cpl.merge(cpl1));
    EXPECT_TRUE(cpl.merge(cpl2));
    EXPECT_EQ(4, cpl.size());

    EXPECT_FALSE(cpl.merge(cpl3));
    EXPECT_EQ(4, cpl.size());
}
