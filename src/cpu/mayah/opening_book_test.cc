#include "opening_book.h"

#include <gtest/gtest.h>

#include "core/plain_field.h"

using namespace std;

TEST(OpeningBookFieldTest, varCount)
{
    OpeningBookField obf1("test",
                          vector<string> {
                              "AAA...",
                          });
    OpeningBookField obf2("test",
                          vector<string> {
                              "..ABBB",
                          });
    OpeningBookField obf3("test",
                          vector<string> {
                              ".*ABBB",
                          });

    EXPECT_EQ(3, obf1.varCount());
    EXPECT_EQ(4, obf2.varCount());

    // We don't count *
    EXPECT_EQ(4, obf3.varCount());

    {
        OpeningBookField f(obf1);
        f.merge(obf2);
        EXPECT_EQ(6, f.varCount());
    }
    {
        OpeningBookField f(obf1);
        f.merge(obf3);
        EXPECT_EQ(6, f.varCount());
    }
}

TEST(OpeningBookFieldTest, match1)
{
    OpeningBookField obf("test",
                         vector<string> {
                             "AAA...",
                         });

    PlainField pf0;
    PlainField pf1("RRR   ");
    PlainField pf2("R     ");
    PlainField pf3("R R   ");

    EXPECT_EQ(OpeningBookField::MatchResult(true, 0, 0, 0), obf.match(pf0));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 3, 3, 0), obf.match(pf1));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 1, 1, 0), obf.match(pf2));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 2, 2, 0), obf.match(pf3));
}

TEST(OpeningBookFieldTest, match2)
{
    OpeningBookField obf("test",
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

    EXPECT_EQ(OpeningBookField::MatchResult(true, 0, 0, 0), obf.match(pf0));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 16, 16, 0), obf.match(pf1));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 16, 16, 0), obf.match(pf2));
}

TEST(OpeningBookFieldTest, match3)
{
    OpeningBookField obf("test",
                         vector<string> {
                             "b.....",
                             "BEExy.",
                             "DDEXYy",
                             "BACXYZ",
                             "BBACXY",
                             "AACCXY",
                         });

    PlainField pf(
        "R..B.B"
        "YYBB.B");

    EXPECT_EQ(OpeningBookField::MatchResult(true, 8, 8, 0), obf.match(pf));
}

TEST(OpeningBookFieldTest, matchWithStar)
{
    OpeningBookField obf("test",
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

    EXPECT_EQ(OpeningBookField::MatchResult(true, 0, 0, 0), obf.match(pf0));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 7, 7, 0), obf.match(pf1));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 7, 7, 0), obf.match(pf2));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 7, 7, 0), obf.match(pf3));
}

TEST(OpeningBookFieldTest, matchWithAllowing)
{
    OpeningBookField obf("test",
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

    EXPECT_EQ(OpeningBookField::MatchResult(true, 0, 0, 0), obf.match(pf0));
    EXPECT_EQ(OpeningBookField::MatchResult(false, 0, 0, 0), obf.match(pf1));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 4, 4, 1), obf.match(pf2));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 3, 3, 1), obf.match(pf3));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 5, 5, 0), obf.match(pf4));
    EXPECT_EQ(OpeningBookField::MatchResult(false, 0, 0, 0), obf.match(pf5));
}

TEST(OpeningBookFieldTest, unmatch1)
{
    OpeningBookField obf("test",
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

    EXPECT_EQ(OpeningBookField::MatchResult(false, 0, 0, 0), obf.match(pf1));
    EXPECT_EQ(OpeningBookField::MatchResult(false, 0, 0, 0), obf.match(pf2));
    EXPECT_EQ(OpeningBookField::MatchResult(false, 0, 0, 0), obf.match(pf3));
    EXPECT_EQ(OpeningBookField::MatchResult(false, 0, 0, 0), obf.match(pf4));
}

TEST(OpeningBookFieldTest, unmatch2)
{
    OpeningBookField obf("test",
                         vector<string> {
                             "..AAA.",
                         });

    PlainField pf1(" B B  ");

    EXPECT_EQ(OpeningBookField::MatchResult(false, 0, 0, 0), obf.match(pf1));
}

TEST(OpeningBookFieldTest, unmatch3)
{
    OpeningBookField obf("test",
                         vector<string> {
                             "AAABBB",
                         });

    PlainField pf("Y    Y");

    EXPECT_EQ(OpeningBookField::MatchResult(false, 0, 0, 0), obf.match(pf));
}

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

    obf.merge(obf2);

    EXPECT_EQ(0, obf.score(6, 6));
    EXPECT_EQ(1, obf.score(1, 4));
    EXPECT_EQ(1, obf.score(1, 5));
    EXPECT_EQ(2, obf.score(1, 1));
    EXPECT_EQ(2, obf.score(6, 1));

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

    EXPECT_EQ(OpeningBookField::MatchResult(true, 0, 0, 0), obf.match(pf0));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 38, 19, 0), obf.match(pf1));

    EXPECT_EQ(OpeningBookField::MatchResult(false, 0, 0, 0), obf.match(pf2));
}
