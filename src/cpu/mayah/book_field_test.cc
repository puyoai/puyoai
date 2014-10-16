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
                      "..ABBB",
                  });

    EXPECT_EQ(3, bf1.varCount());
    EXPECT_EQ(4, bf2.varCount());

    bf1.merge(bf2);
    EXPECT_EQ(6, bf1.varCount());
}

TEST(BookFieldTest, match1)
{
    BookField bf("test",
                 vector<string> {
                     "AAA...",
                 });

    PlainField pf0;
    PlainField pf1("RRR   ");
    PlainField pf2("R     ");
    PlainField pf3("R R   ");

    EXPECT_EQ(BookField::MatchResult(true, 0, 0), bf.match(pf0));
    EXPECT_EQ(BookField::MatchResult(true, 3, 3), bf.match(pf1));
    EXPECT_EQ(BookField::MatchResult(true, 1, 1), bf.match(pf2));
    EXPECT_EQ(BookField::MatchResult(true, 2, 2), bf.match(pf3));
}

TEST(BookFieldTest, match2)
{
    BookField bf("test",
                 vector<string> {
                     "BAD.C.",
                     "BBADDD",
                     "AACCCX",
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

    EXPECT_EQ(BookField::MatchResult(true, 0, 0), bf.match(pf0));
    EXPECT_EQ(BookField::MatchResult(true, 16, 16), bf.match(pf1));
    EXPECT_EQ(BookField::MatchResult(true, 16, 16), bf.match(pf2));
}

TEST(BookFieldTest, unmatch1)
{
    BookField bf("test",
                 vector<string> {
                     "BAD.C.",
                     "BBADDD",
                     "AACCCX",
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

    EXPECT_EQ(BookField::MatchResult(false, 0, 0), bf.match(pf1));
    EXPECT_EQ(BookField::MatchResult(false, 0, 0), bf.match(pf2));
    EXPECT_EQ(BookField::MatchResult(false, 0, 0), bf.match(pf3));
    EXPECT_EQ(BookField::MatchResult(false, 0, 0), bf.match(pf4));
}

TEST(BookFieldTest, unmatch2)
{
    BookField bf("test",
                 vector<string> {
                     "..AAA.",
                 });

    PlainField pf1(" B B  ");

    EXPECT_EQ(BookField::MatchResult(false, 0, 0), bf.match(pf1));
}

TEST(BookFieldTest, unmatch3)
{
    BookField bf("test",
                 vector<string> {
                     "AAABBB",
                 });

    PlainField pf("Y    Y");

    EXPECT_EQ(BookField::MatchResult(false, 0, 0), bf.match(pf));
}

TEST(BookFieldTest, merge)
{
    BookField bf("test",
                 vector<string> {
                     "BX....",
                     "XX....",
                     "BAD.C.",
                     "BBADDD",
                     "AACCCE",
                 }, 1);

    BookField bf2("test2",
                  vector<string> {
                      "....E.",
                      "BADECE",
                      "BBADDD",
                      "AACCCE",
                 }, 2);

    bf.merge(bf2);

    EXPECT_EQ(0, bf.score(6, 6));
    EXPECT_EQ(1, bf.score(1, 4));
    EXPECT_EQ(1, bf.score(1, 5));
    EXPECT_EQ(2, bf.score(1, 1));
    EXPECT_EQ(2, bf.score(6, 1));

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

    EXPECT_EQ(BookField::MatchResult(true, 0, 0), bf.match(pf0));
    EXPECT_EQ(BookField::MatchResult(true, 38, 19), bf.match(pf1));

    EXPECT_EQ(BookField::MatchResult(false, 0, 0), bf.match(pf2));
}
