#include "book_field.h"

#include <gtest/gtest.h>

using namespace std;

TEST(BookFieldTest, varCount)
{
    BookField bf1("test",
                  vector<string> {
                      "AAA...",
                  });

    BookField bf2("test",
                  vector<string> {
                      "BAD.C.",
                      "BBADDD",
                      "AACCC.",
                  });

    EXPECT_EQ(3, bf1.totalVariableCount());
    EXPECT_EQ(15, bf2.totalVariableCount());
}

TEST(BookFieldTest, matchEasy)
{
    BookField bf("test",
                 vector<string> {
                     "AAA...",
                 });

    PlainField pf0;
    PlainField pf1("RRR   ");
    PlainField pf2("R     ");
    PlainField pf3("R R   ");

    EXPECT_EQ(0, bf.matchCount(pf0));
    EXPECT_EQ(3, bf.matchCount(pf1));
    EXPECT_EQ(1, bf.matchCount(pf2));
    EXPECT_EQ(2, bf.matchCount(pf3));
}

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

    EXPECT_EQ(0,  bf.matchCount(pf0));
    EXPECT_EQ(15, bf.matchCount(pf1));
    EXPECT_EQ(15, bf.matchCount(pf2));
}

TEST(BookFieldTest, unmatch1)
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

    EXPECT_EQ(0, bf.matchCount(pf1));
    EXPECT_EQ(0, bf.matchCount(pf2));
    EXPECT_EQ(0, bf.matchCount(pf3));
    EXPECT_EQ(0, bf.matchCount(pf4));
}

TEST(BookFieldTest, unmatch2)
{
    BookField bf("test",
                 vector<string> {
                     "..AAA.",
                 });

    PlainField pf1(" B B  ");

    EXPECT_EQ(0, bf.matchCount(pf1));
}

TEST(BookFieldTest, merge)
{
    BookField bf("test",
                 vector<string> {
                     "BAD.C.",
                     "BBADDD",
                     "AACCC.",
                 });

    BookField bf2("test2",
                  vector<string> {
                      "....E.",
                      "BADECE",
                      "BBADDD",
                      "AACCCE",
                  });

    bf.merge(bf2);

    PlainField pf0;

    PlainField pf1(
        "    B "
        "BYGBRB"
        "BBYGGG"
        "YYRRRB");

    PlainField pf2(
        "    GG"
        "BYBGRG"
        "BBYBBB"
        "YYRRRG");

    EXPECT_EQ(0, bf.matchCount(pf0));
    EXPECT_EQ(19, bf.matchCount(pf1));
    EXPECT_EQ(0, bf.matchCount(pf2));
}
