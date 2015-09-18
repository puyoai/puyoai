#include "core/probability/column_puyo_list_probability.h"

#include <gtest/gtest.h>

TEST(ColumnPuyoListProbabilityTest, necessaryPuyosWithColumnPuyoList)
{
    const ColumnPuyoListProbability* instance = ColumnPuyoListProbability::instanceSlow();

    ColumnPuyoList cpl;
    EXPECT_DOUBLE_EQ(0, instance->necessaryKumipuyos(cpl));

    cpl.add(1, PuyoColor::RED);
    EXPECT_DOUBLE_EQ(16.0 / 7, instance->necessaryKumipuyos(cpl));

    cpl.add(2, PuyoColor::RED);
    EXPECT_DOUBLE_EQ(13.0 * 16 / 49, instance->necessaryKumipuyos(cpl));
}
