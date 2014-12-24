#include "opening_book.h"

#include <gtest/gtest.h>

#include "core/core_field.h"

using namespace std;

TEST(OpeningBookFieldTest, merge)
{
    OpeningBookField obf("test",
                         vector<string> {
                             "BX....",
                             "XX....",
                             "BAD.C.",
                             "BBADDD",
                             "AACCCE",
                         }, 1);

    OpeningBookField obf2("test2",
                          vector<string> {
                              "....E.",
                              "BADECE",
                              "BBADDD",
                              "AACCCE",
                          }, 2);

    ASSERT_TRUE(obf.merge(obf2));

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

    EXPECT_EQ(PatternMatchResult(true, 0, 0, 0), obf.match(f0));
    EXPECT_EQ(PatternMatchResult(true, 38, 19, 0), obf.match(f1));
    EXPECT_EQ(PatternMatchResult(false, 0, 0, 0), obf.match(f2));
}
