#include "core/column_puyo_list.h"

#include <gtest/gtest.h>

TEST(ColumnPuyoListTest, constructor1)
{
    ColumnPuyoList cpl;
    EXPECT_EQ(0, cpl.size());
    EXPECT_TRUE(cpl.isEmpty());
}

TEST(ColumnPuyoListTest, constructor2)
{
    ColumnPuyoList cpl(3, PuyoColor::RED, 2);
    EXPECT_EQ(2, cpl.size());
    EXPECT_FALSE(cpl.isEmpty());
}

TEST(ColumnPuyoListTest, append)
{
    ColumnPuyoList cpl;
    ColumnPuyoList cpl1(3, PuyoColor::RED, 2);
    ColumnPuyoList cpl2(3, PuyoColor::BLUE, 2);
    ColumnPuyoList cpl3(3, PuyoColor::BLUE, ColumnPuyoList::MAX_SIZE);

    EXPECT_TRUE(cpl.append(cpl1));
    EXPECT_TRUE(cpl.append(cpl2));
    EXPECT_EQ(4, cpl.size());

    EXPECT_FALSE(cpl.append(cpl3));
    EXPECT_EQ(4, cpl.size());
}
