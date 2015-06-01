#include "core/algorithm/field_pattern.h"

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

TEST(FieldPatternTest, varCount)
{
    FieldPattern pattern1("AAA...");
    FieldPattern pattern2("..AAAB");
    FieldPattern pattern3(".*ABBB");

    EXPECT_EQ(3, pattern1.numVariables());
    EXPECT_EQ(4, pattern2.numVariables());
    EXPECT_EQ(4, pattern3.numVariables()); // We don't count *
}

TEST(FieldPatternTest, mirror)
{
    FieldPattern pattern("Aa*...");
    pattern.setMustVar(1, 1);

    FieldPattern mirror = pattern.mirror();

    EXPECT_EQ(1, pattern.numVariables());
    EXPECT_EQ(1, mirror.numVariables());

    EXPECT_EQ(FieldBits("1....."), pattern.mustPatternBits());
    EXPECT_EQ(FieldBits(".....1"), mirror.mustPatternBits());

    EXPECT_EQ(FieldBits("..1..."), pattern.anyPatternBits());
    EXPECT_EQ(FieldBits("...1.."), mirror.anyPatternBits());

    EXPECT_EQ(FieldBits("1....."), pattern.pattern(0).varBits);
    EXPECT_EQ(FieldBits(".....1"), mirror.pattern(0).varBits);

    EXPECT_EQ(FieldBits(".1...."), pattern.pattern(0).allowVarBits);
    EXPECT_EQ(FieldBits("....1."), mirror.pattern(0).allowVarBits);
}
