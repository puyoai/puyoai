#include "book_field.h"

#include <gtest/gtest.h>

using namespace std;

TEST(BookFieldTest, match)
{
    BookField bf("test",
                 vector<string> {
                     "BAD.C.",
                     "BBADDD",
                     "AACCC.",
                 });

    PlainField pf0;

    PlainField pf1(
        "BYGBRB"
        "BBYGGG"
        "YYRRRB");

    PlainField pf2(
        "BYB R "
        "BBYBBB"
        "YYRRRG");

    EXPECT_TRUE(bf.matches(pf0));
    EXPECT_TRUE(bf.matches(pf1));
    EXPECT_TRUE(bf.matches(pf2));
}

TEST(BookFieldTest, unmatch)
{
    BookField bf("test",
                 vector<string> {
                     "BAD.C.",
                     "BBADDD",
                     "AACCC.",
                 });

    PlainField pf1(
        "B     "
        "BYGBRB"
        "BBYGGG"
        "YYRRRB");

    PlainField pf2(
        "    R "
        "BYB R "
        "BBYBBB"
        "YYRRRG");

    PlainField pf3(
        "BYO R "
        "BBYOOO"
        "YYRRRY");

    PlainField pf4(
        "BRB R "
        "BBRBBB"
        "RRRRRY");

    EXPECT_FALSE(bf.matches(pf1));
    EXPECT_FALSE(bf.matches(pf2));
    EXPECT_FALSE(bf.matches(pf3));
    EXPECT_FALSE(bf.matches(pf4));
}
