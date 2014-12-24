#include "opening_book.h"

#include <gtest/gtest.h>

#include "core/core_field.h"

using namespace std;

TEST(OpeningBookFieldTest, merge)
{
    PatternField pf1(
        "BX...."
        "XX...."
        "BAD.C."
        "BBADDD"
        "AACCCE", 1);
    PatternField pf2(
        "....E."
        "BADECE"
        "BBADDD"
        "AACCCE", 2);

    PatternField pf;
    ASSERT_TRUE(PatternField::merge(pf1, pf2, &pf));

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

    OpeningBookField obf("test", pf);
    EXPECT_EQ(PatternMatchResult(true, 0, 0, 0), obf.match(f0));
    EXPECT_EQ(PatternMatchResult(true, 38, 19, 0), obf.match(f1));
    EXPECT_EQ(PatternMatchResult(false, 0, 0, 0), obf.match(f2));
}
