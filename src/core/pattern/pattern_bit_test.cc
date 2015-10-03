#include "core/pattern/pattern_bit.h"

#include <algorithm>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "core/column_puyo.h"
#include "core/column_puyo_list.h"
#include "core/core_field.h"
#include "core/field_checker.h"
#include "core/position.h"

using namespace std;

TEST(PatternBitTest, put)
{
    PatternBitSet pbs;

    FieldBits var1(
        "1....."
        "111...");
    FieldBits notVar1(
        "1....."
        ".11..."
        "...1..");
    PatternBit bit1(var1, notVar1);

    FieldBits var2(
        "......"
        "1111..");
    FieldBits notVar2(
        "1111.."
        "....1.");
    PatternBit bit2(var2, notVar2);

    EXPECT_EQ(0, pbs.put(bit1));
    EXPECT_EQ(1, pbs.put(bit2));

    // Even the same pattern bit set is put, PatternBitSet should return the same id.
    EXPECT_EQ(0, pbs.put(bit1));
    EXPECT_EQ(1, pbs.put(bit2));

    EXPECT_EQ(2UL, pbs.size());
}
