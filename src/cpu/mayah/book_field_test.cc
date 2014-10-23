#include "book_field.h"

#include <gtest/gtest.h>

#include "core/plain_field.h"

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
    BookField bf3("test",
                  vector<string> {
                      ".*ABBB",
                  });

    EXPECT_EQ(3, bf1.varCount());
    EXPECT_EQ(4, bf2.varCount());

    // We don't count *
    EXPECT_EQ(4, bf3.varCount());

    {
        BookField f(bf1);
        f.merge(bf2);
        EXPECT_EQ(6, f.varCount());
    }
    {
        BookField f(bf1);
        f.merge(bf3);
        EXPECT_EQ(6, f.varCount());
    }
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

    EXPECT_EQ(BookField::MatchResult(true, 0, 0, 0), bf.match(pf0));
    EXPECT_EQ(BookField::MatchResult(true, 3, 3, 0), bf.match(pf1));
    EXPECT_EQ(BookField::MatchResult(true, 1, 1, 0), bf.match(pf2));
    EXPECT_EQ(BookField::MatchResult(true, 2, 2, 0), bf.match(pf3));
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

    EXPECT_EQ(BookField::MatchResult(true, 0, 0, 0), bf.match(pf0));
    EXPECT_EQ(BookField::MatchResult(true, 16, 16, 0), bf.match(pf1));
    EXPECT_EQ(BookField::MatchResult(true, 16, 16, 0), bf.match(pf2));
}

TEST(BookFieldTest, matchWithStar)
{
    BookField bf("test",
                 vector<string> {
                     ".***CC",
                     ".AABBB"
                 });

    PlainField pf0;

    PlainField pf1(
        ".YYYGG"
        ".RRBBB");

    PlainField pf2(
        ".B...."
        ".BBYYY"
        ".RRBBB");

    PlainField pf3(
        ".GGGYY"
        ".RRBBB");

    EXPECT_EQ(BookField::MatchResult(true, 0, 0, 0), bf.match(pf0));
    EXPECT_EQ(BookField::MatchResult(true, 7, 7, 0), bf.match(pf1));
    EXPECT_EQ(BookField::MatchResult(true, 7, 7, 0), bf.match(pf2));
    EXPECT_EQ(BookField::MatchResult(true, 7, 7, 0), bf.match(pf3));
}

TEST(BookFieldTest, matchWithAllowing)
{
    BookField bf("test",
                 vector<string> {
                     "..C...",
                     "AAaBB."
                 });

    PlainField pf0;

    PlainField pf1(
        "RRBBB.");

    PlainField pf2(
        "RRRBB.");

    PlainField pf3(
        "R.RBB.");

    PlainField pf4(
        "..R..."
        "RRYRR.");

    PlainField pf5(
        "RRRRR.");

    EXPECT_EQ(BookField::MatchResult(true, 0, 0, 0), bf.match(pf0));
    EXPECT_EQ(BookField::MatchResult(false, 0, 0, 0), bf.match(pf1));
    EXPECT_EQ(BookField::MatchResult(true, 4, 4, 1), bf.match(pf2));
    EXPECT_EQ(BookField::MatchResult(true, 3, 3, 1), bf.match(pf3));
    EXPECT_EQ(BookField::MatchResult(true, 5, 5, 0), bf.match(pf4));
    EXPECT_EQ(BookField::MatchResult(false, 0, 0, 0), bf.match(pf5));
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

    EXPECT_EQ(BookField::MatchResult(false, 0, 0, 0), bf.match(pf1));
    EXPECT_EQ(BookField::MatchResult(false, 0, 0, 0), bf.match(pf2));
    EXPECT_EQ(BookField::MatchResult(false, 0, 0, 0), bf.match(pf3));
    EXPECT_EQ(BookField::MatchResult(false, 0, 0, 0), bf.match(pf4));
}

TEST(BookFieldTest, unmatch2)
{
    BookField bf("test",
                 vector<string> {
                     "..AAA.",
                 });

    PlainField pf1(" B B  ");

    EXPECT_EQ(BookField::MatchResult(false, 0, 0, 0), bf.match(pf1));
}

TEST(BookFieldTest, unmatch3)
{
    BookField bf("test",
                 vector<string> {
                     "AAABBB",
                 });

    PlainField pf("Y    Y");

    EXPECT_EQ(BookField::MatchResult(false, 0, 0, 0), bf.match(pf));
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

    EXPECT_EQ(BookField::MatchResult(true, 0, 0, 0), bf.match(pf0));
    EXPECT_EQ(BookField::MatchResult(true, 38, 19, 0), bf.match(pf1));

    EXPECT_EQ(BookField::MatchResult(false, 0, 0, 0), bf.match(pf2));
}
