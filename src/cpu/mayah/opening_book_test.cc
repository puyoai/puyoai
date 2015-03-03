#include "opening_book.h"

#include <gtest/gtest.h>

#include "core/core_field.h"

using namespace std;

TEST(OpeningBookFieldTest, merge)
{
    FieldPattern pattern1(
        "BX...."
        "XX...."
        "BAD.C."
        "BBADDD"
        "AACCCE", 1);
    FieldPattern pattern2(
        "....E."
        "BADECE"
        "BBADDD"
        "AACCCE", 2);

    FieldPattern pattern;
    ASSERT_TRUE(FieldPattern::merge(pattern1, pattern2, &pattern));

    CoreField f0;
    CoreField f1(
        "    B "
        "BYGBRB"
        "BBYGGG"
        "YYRRRB");
    CoreField f2(
        "    GG"
        "BYBGRG"
        "BBYBBB"
        "YYRRRG");

    OpeningBookField obf("test", pattern);
    EXPECT_EQ(PatternMatchResult(true, 0, 0, 0, {'A', 'B', 'C', 'D', 'E', 'X'}), obf.match(f0));
    EXPECT_EQ(PatternMatchResult(true, 38, 19, 0, {'X'}), obf.match(f1));
    EXPECT_EQ(PatternMatchResult(false, 0, 0, 0), obf.match(f2));
}
