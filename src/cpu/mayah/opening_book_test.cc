#include "opening_book.h"

#include <gtest/gtest.h>

#include "core/core_field.h"

using namespace std;

TEST(OpeningBookFieldTest, match1)
{
    OpeningBookField obf("test",
                         vector<string> {
                             "AAA...",
                         });

    CoreField f0;
    CoreField f1("RRR   ");
    CoreField f2("R     ");
    CoreField f3("R R   ");

    EXPECT_EQ(OpeningBookField::MatchResult(true, 0, 0, 0), obf.match(f0));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 3, 3, 0), obf.match(f1));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 1, 1, 0), obf.match(f2));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 2, 2, 0), obf.match(f3));
}

TEST(OpeningBookFieldTest, match2)
{
    OpeningBookField obf("test",
                         vector<string> {
                             "BAD.C.",
                             "BBADDD",
                             "AACCCX",
                         });

    CoreField f0;

    CoreField f1(
        "BYGBRB"
        "BBYGGG"
        "YYRRRB");

    CoreField f2(
        "BYB R "
        "BBYBBB"
        "YYRRRG");

    EXPECT_EQ(OpeningBookField::MatchResult(true, 0, 0, 0), obf.match(f0));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 16, 16, 0), obf.match(f1));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 16, 16, 0), obf.match(f2));
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

    CoreField f(
        "R..B.B"
        "YYBB.B");

    EXPECT_EQ(OpeningBookField::MatchResult(true, 8, 8, 0), obf.match(f));
}

TEST(OpeningBookFieldTest, matchWithStar)
{
    OpeningBookField obf("test",
                         vector<string> {
                             ".***CC",
                             ".AABBB"
                         });

    CoreField f0;

    CoreField f1(
        ".YYYGG"
        ".RRBBB");

    CoreField f2(
        ".B...."
        ".BBYYY"
        ".RRBBB");

    CoreField f3(
        ".GGGYY"
        ".RRBBB");

    EXPECT_EQ(OpeningBookField::MatchResult(true, 0, 0, 0), obf.match(f0));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 7, 7, 0), obf.match(f1));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 7, 7, 0), obf.match(f2));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 7, 7, 0), obf.match(f3));
}

TEST(OpeningBookFieldTest, matchWithAllowing)
{
    OpeningBookField obf("test",
                         vector<string> {
                             "..C...",
                             "AAaBB."
                         });

    CoreField f0;

    CoreField f1(
        "RRBBB.");

    CoreField f2(
        "RRRBB.");

    CoreField f3(
        "R.RBB.");

    CoreField f4(
        "..R..."
        "RRYRR.");

    CoreField f5(
        "RRRRR.");

    EXPECT_EQ(OpeningBookField::MatchResult(true, 0, 0, 0), obf.match(f0));
    EXPECT_EQ(OpeningBookField::MatchResult(false, 0, 0, 0), obf.match(f1));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 4, 4, 1), obf.match(f2));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 3, 3, 1), obf.match(f3));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 5, 5, 0), obf.match(f4));
    EXPECT_EQ(OpeningBookField::MatchResult(false, 0, 0, 0), obf.match(f5));
}

TEST(OpeningBookFieldTest, unmatch1)
{
    OpeningBookField obf("test",
                         vector<string> {
                             "BAD.C.",
                             "BBADDD",
                             "AACCCX",
                         });

    CoreField f1(
        "B     "
        "BYGBRB"
        "BBYGGG"
        "YYRRRB");

    CoreField f2(
        "    R "
        "BYB R "
        "BBYBBB"
        "YYRRRG");

    CoreField f3(
        "BYO R "
        "BBYOOO"
        "YYRRRY");

    CoreField f4(
        "BRB R "
        "BBRBBB"
        "RRRRRY");

    EXPECT_EQ(OpeningBookField::MatchResult(false, 0, 0, 0), obf.match(f1));
    EXPECT_EQ(OpeningBookField::MatchResult(false, 0, 0, 0), obf.match(f2));
    EXPECT_EQ(OpeningBookField::MatchResult(false, 0, 0, 0), obf.match(f3));
    EXPECT_EQ(OpeningBookField::MatchResult(false, 0, 0, 0), obf.match(f4));
}

TEST(OpeningBookFieldTest, unmatch2)
{
    OpeningBookField obf("test",
                         vector<string> {
                             "..AAA.",
                         });

    CoreField f1(" B B  ");

    EXPECT_EQ(OpeningBookField::MatchResult(false, 0, 0, 0), obf.match(f1));
}

TEST(OpeningBookFieldTest, unmatch3)
{
    OpeningBookField obf("test",
                         vector<string> {
                             "AAABBB",
                         });

    CoreField f("Y    Y");

    EXPECT_EQ(OpeningBookField::MatchResult(false, 0, 0, 0), obf.match(f));
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

    EXPECT_EQ(OpeningBookField::MatchResult(true, 0, 0, 0), obf.match(f0));
    EXPECT_EQ(OpeningBookField::MatchResult(true, 38, 19, 0), obf.match(f1));
    EXPECT_EQ(OpeningBookField::MatchResult(false, 0, 0, 0), obf.match(f2));
}
