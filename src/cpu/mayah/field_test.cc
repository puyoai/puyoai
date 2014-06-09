#include "field.h"

#include <gtest/gtest.h>
#include <string>

#include "core/algorithm/rensa_info.h"
#include "core/constant.h"
#include "core/decision.h"
#include "core/kumipuyo.h"

using namespace std;

TEST(FieldTest, SetAndGet)
{
    ArbitrarilyModifiableField f;

    f.setPuyo(1, 1, RED);
    f.setPuyo(1, 2, BLUE);
    f.setPuyo(1, 3, YELLOW);
    f.setPuyo(1, 4, GREEN);
    EXPECT_EQ(RED, f.color(1, 1));
    EXPECT_EQ(BLUE, f.color(1, 2));
    EXPECT_EQ(YELLOW, f.color(1, 3));
    EXPECT_EQ(GREEN, f.color(1, 4));

    f.setPuyo(1, 4, EMPTY);
    EXPECT_EQ(RED, f.color(1, 1));
    EXPECT_EQ(BLUE, f.color(1, 2));
    EXPECT_EQ(YELLOW, f.color(1, 3));
    EXPECT_EQ(EMPTY, f.color(1, 4));

    f.setPuyo(1, 1, EMPTY);
    f.setPuyo(1, 2, EMPTY);
    f.setPuyo(1, 3, EMPTY);
    EXPECT_EQ(EMPTY, f.color(1, 1));
    EXPECT_EQ(EMPTY, f.color(1, 2));
    EXPECT_EQ(EMPTY, f.color(1, 3));
    EXPECT_EQ(EMPTY, f.color(1, 4));
}

TEST(FieldTest, IsZenkeshi)
{
    Field f1;
    Field f2("400000");
    Field f3("100000"); // When there is an ojama, it should not be zenkeshi.

    EXPECT_TRUE(f1.isZenkeshi());
    EXPECT_FALSE(f2.isZenkeshi());
    EXPECT_FALSE(f3.isZenkeshi());
}

void testUrl(string url, int expected_chains, int expected_score)
{
    Field f(url);
    BasicRensaResult rensaInfo = f.simulate();
    EXPECT_EQ(expected_chains, rensaInfo.chains);
    EXPECT_EQ(expected_score, rensaInfo.score);
}

TEST(FieldTest, ConnectedPuyoNums)
{
    Field f("004455"
            "045465");

    EXPECT_EQ(3, f.connectedPuyoNums(3, 2));
    EXPECT_EQ(3, f.connectedPuyoNums(5, 2));
    EXPECT_EQ(1, f.connectedPuyoNums(5, 1));
}

TEST(FieldTest, ConnectedPuyoNumsWithAllowingOnePointJump1)
{
    Field f("707707"
            "404404"
            "404555");

    EXPECT_EQ(5, f.connectedPuyoNumsWithAllowingOnePointJump(1, 1).first);
    EXPECT_EQ(1, f.connectedPuyoNumsWithAllowingOnePointJump(1, 1).second);

    EXPECT_EQ(6, f.connectedPuyoNumsWithAllowingOnePointJump(3, 1).first);
    EXPECT_EQ(2, f.connectedPuyoNumsWithAllowingOnePointJump(3, 1).second);

    EXPECT_EQ(3, f.connectedPuyoNumsWithAllowingOnePointJump(1, 3).first);
    EXPECT_EQ(1, f.connectedPuyoNumsWithAllowingOnePointJump(1, 3).second);

    EXPECT_EQ(4, f.connectedPuyoNumsWithAllowingOnePointJump(3, 3).first);
    EXPECT_EQ(2, f.connectedPuyoNumsWithAllowingOnePointJump(3, 3).second);
}

TEST(FieldTest, ConnectedPuyoNumsWithAllowingOnePointJump2)
{
    Field f("000000"
            "004500"
            "445505");

    EXPECT_EQ(2, f.connectedPuyoNumsWithAllowingOnePointJump(1, 1).first);
    EXPECT_EQ(0, f.connectedPuyoNumsWithAllowingOnePointJump(1, 1).second);
}

TEST(FieldTest, FindBestBreathingSpace1)
{
    Field f("546700"
            "554677"
            "446675");

    int breathingX, breathingY;
    EXPECT_TRUE(f.findBestBreathingSpace(breathingX, breathingY, 1, 2));
    EXPECT_EQ(1, breathingX);
    EXPECT_EQ(4, breathingY);

    EXPECT_FALSE(f.findBestBreathingSpace(breathingX, breathingY, 1, 1));
}

TEST(FieldTest, FindBestBreathingSpace2)
{
    Field f("406060"
            "455055"
            "454045");

    int breathingX, breathingY;
    EXPECT_TRUE(f.findBestBreathingSpace(breathingX, breathingY, 2, 1));
    EXPECT_EQ(2, breathingX);
    EXPECT_EQ(3, breathingY);

    EXPECT_TRUE(f.findBestBreathingSpace(breathingX, breathingY, 6, 1));
    EXPECT_EQ(6, breathingX);
    EXPECT_EQ(3, breathingY);
}

TEST(FieldTest, CountPuyos)
{
    Field f("050015"
            "050055"
            "445644"
            "445644");

    EXPECT_EQ(18, f.countPuyos());
}

TEST(FieldTest, CountColorPuyos)
{
    Field f("050015"
            "050055"
            "445644"
            "445644");

    EXPECT_EQ(17, f.countColorPuyos());
}

TEST(FieldTest, DropKumipuyoExtreme)
{
    Field f1("http://www.inosendo.com/puyo/rensim/??110111550477450455745466754655664576755776666564777644555455666545775454444554");
    Field f2("http://www.inosendo.com/puyo/rensim/??110111550477451455745466754655664576755776666564777644555455666545775454444554");

    f1.dropKumipuyoSafely(Decision(3, 0), Kumipuyo(RED, RED));
    f1.dropKumipuyoSafely(Decision(3, 0), Kumipuyo(RED, RED));

    f2.dropKumipuyoSafely(Decision(3, 0), Kumipuyo(RED, RED));
    f2.dropKumipuyoSafely(Decision(3, 0), Kumipuyo(RED, RED));
}
