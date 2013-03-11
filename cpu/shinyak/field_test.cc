#include "field.h"

#include <gtest/gtest.h>
#include <string>

#include "core/constant.h"
#include "core/decision.h"
#include "rensa_info.h"

using namespace std;

TEST(FieldTest, Initial)
{
    Field f;

    for (int x = 1; x <= Field::WIDTH; ++x) {
        for (int y = 1; y <= Field::HEIGHT; ++y) {
            EXPECT_EQ(EMPTY, f.color(x, y)) << x << ' ' << y;
        }
    }
}

TEST(FieldTest, Color)
{
    Field f("444000");

    EXPECT_EQ(WALL, f.color(0, 1));
    EXPECT_EQ(RED, f.color(1, 1));
    EXPECT_EQ(RED, f.color(2, 1));
    EXPECT_EQ(RED, f.color(3, 1));
    EXPECT_EQ(EMPTY, f.color(4, 1));
    EXPECT_EQ(EMPTY, f.color(5, 1));
    EXPECT_EQ(EMPTY, f.color(6, 1));
    EXPECT_EQ(WALL, f.color(7, 1));
}

TEST(FieldTest, SetAndGet)
{
    ArbitrarilyModifiableField f;

    f.setColor(1, 1, RED);
    f.setColor(1, 2, BLUE);
    f.setColor(1, 3, YELLOW);
    f.setColor(1, 4, GREEN);
    EXPECT_EQ(RED, f.color(1, 1));
    EXPECT_EQ(BLUE, f.color(1, 2));
    EXPECT_EQ(YELLOW, f.color(1, 3));
    EXPECT_EQ(GREEN, f.color(1, 4));

    f.setColor(1, 4, EMPTY);
    EXPECT_EQ(RED, f.color(1, 1));
    EXPECT_EQ(BLUE, f.color(1, 2));
    EXPECT_EQ(YELLOW, f.color(1, 3));
    EXPECT_EQ(EMPTY, f.color(1, 4));

    f.setColor(1, 1, EMPTY);
    f.setColor(1, 2, EMPTY);
    f.setColor(1, 3, EMPTY);
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

TEST(FieldTest, ForceDrop)
{
    Field f("544555"
            "000000"
            "464646"
            "000000");

    f.forceDrop();

    EXPECT_EQ(2, f.height(1));
    EXPECT_EQ(2, f.height(2));
    EXPECT_EQ(2, f.height(3));
    EXPECT_EQ(2, f.height(4));
    EXPECT_EQ(2, f.height(5));
    EXPECT_EQ(2, f.height(6));

    EXPECT_EQ(RED, f.color(1, 1));
    EXPECT_EQ(BLUE, f.color(1, 2));
    EXPECT_EQ(EMPTY, f.color(1, 3));
    EXPECT_EQ(EMPTY, f.color(1, 4));
}

void testUrl(string url, int expected_chains, int expected_score)
{
    Field f(url);
    BasicRensaInfo rensaInfo;
    f.simulate(rensaInfo);
    EXPECT_EQ(expected_chains, rensaInfo.chains);
    EXPECT_EQ(expected_score, rensaInfo.score);
}

TEST(FieldTest, ChainAndScoreTest1)
{
    testUrl("http://www.inosendo.com/puyo/rensim/??444400", 1, 40);
}

TEST(FieldTest, ChainAndScoreTest2)
{
    testUrl("http://www.inosendo.com/puyo/rensim/??5000005565444455", 2, 700);
}

TEST(FieldTest, ChainAndScoreTest3)
{
    testUrl("http://www.inosendo.com/puyo/rensim/??50745574464446676456474656476657564547564747676466766747674757644657575475755", 19, 175080);
    testUrl("http://www.inosendo.com/puyo/rensim/??500467767675744454754657447767667644674545455767477644457474656455446775455646", 19, 175080);
    testUrl("http://www.inosendo.com/puyo/rensim/??550050455045451045745074745074645067674067674056567056567515167444416555155", 2, 38540);
    testUrl("http://www.inosendo.com/puyo/rensim/??50550040455075451075745064745064645067674057674747574776567675156644415555155", 3, 43260);
    testUrl("http://www.inosendo.com/puyo/rensim/??550040455075451775745464745464645467674457674147574776567675156644415555155", 4, 50140);
    testUrl("http://www.inosendo.com/puyo/rensim/??745550576455666451175745564745564745567674157674747574776566615156644415555155", 5, 68700);
    testUrl("http://www.inosendo.com/puyo/rensim/??444411114141414114114111414144411114414111114414411114441114111141444141111141", 4, 4840);
    testUrl("http://www.inosendo.com/puyo/rensim/??545544544454454545454545454545545454445544554455454545545454554544445455455445", 9, 49950);
    testUrl("http://www.inosendo.com/puyo/rensim/??444446544611446164564441546166565615454551441444111111111111111111111111111111", 9, 32760);
    testUrl("http://www.inosendo.com/puyo/rensim/??667547466555661677471475451447461666661547457477556446776555744646476466744555", 18, 155980);
    testUrl("http://www.inosendo.com/puyo/rensim/??444044144414114144411411414144141414414141441414114411441144414141141414144144", 11, 47080);
    testUrl("http://www.inosendo.com/puyo/rensim/??444444444444444444444444444444444444444444444444444444444444444444444444", 1, 7200);
}

TEST(FieldTest, FramesTest) {
    BasicRensaInfo info;
    {
        // 1 Rensa, no drop.
        Field f("444400");
        f.simulate(info);
        EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_AFTER_NO_DROP, info.frames);
    }
    {
        Field f("500000"
                "444400");
        f.simulate(info);
        EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE + FRAMES_AFTER_DROP, info.frames);
    }
    {
        Field f("500000"
                "400000"
                "444000");
        f.simulate(info);
        EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP,
                  info.frames);
    }
    {
        Field f("500000"
                "450000"
                "444000");
        f.simulate(info);
        EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP,
                  info.frames);
    }
    {
        Field f("500000"
                "455000"
                "444500");
        f.simulate(info);
        EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP +
                  FRAMES_AFTER_VANISH + FRAMES_AFTER_NO_DROP,
                  info.frames);
    }
    {
        Field f("560000"
                "455000"
                "444500");
        f.simulate(info);
        EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP +
                  FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE + FRAMES_AFTER_DROP,
                  info.frames);
    }
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

TEST(FieldTest, Height)
{
    Field f("004100"
            "004040"
            "004707"
            "014040");

    EXPECT_EQ(0, f.height(1));
    EXPECT_EQ(1, f.height(2));
    EXPECT_EQ(4, f.height(3));
    EXPECT_EQ(4, f.height(4));
    EXPECT_EQ(3, f.height(5));
    EXPECT_EQ(2, f.height(6));
}

TEST(FieldTest, HeightShouldBeCopied)
{
    Field f("004100"
            "004040"
            "004707"
            "014040");

    Field g(f);

    EXPECT_EQ(0, g.height(1));
    EXPECT_EQ(1, g.height(2));
    EXPECT_EQ(4, g.height(3));
    EXPECT_EQ(4, g.height(4));
    EXPECT_EQ(3, g.height(5));
    EXPECT_EQ(2, g.height(6));
}

TEST(FieldTest, HeightAfterSimulate)
{
    Field f("050005"
            "050055"
            "445644"
            "445644");

    BasicRensaInfo info;
    f.simulate(info);

    EXPECT_EQ(0, f.height(1));
    EXPECT_EQ(0, f.height(2));
    EXPECT_EQ(0, f.height(3));
    EXPECT_EQ(2, f.height(4));
    EXPECT_EQ(1, f.height(5));
    EXPECT_EQ(2, f.height(6));
}

TEST(FieldTest, HeightAfterSimulate2)
{
    Field f("450005"
            "445665"
            "556455");

    BasicRensaInfo info;
    f.simulate(info);

    EXPECT_EQ(3, f.height(1));
    EXPECT_EQ(3, f.height(2));
    EXPECT_EQ(2, f.height(3));
    EXPECT_EQ(2, f.height(4));
    EXPECT_EQ(1, f.height(5));
    EXPECT_EQ(0, f.height(6));
}

TEST(FieldTest, DropPuyoOn)
{
    Field f("050005"
            "050055"
            "445644"
            "445644");

    f.dropPuyoOn(1, RED);

    EXPECT_EQ(RED, f.color(1, 3));
    EXPECT_EQ(3, f.height(1));
}

TEST(FieldTest, RemoveTopPuyoFrom)
{
    Field f("456756");

    f.removeTopPuyoFrom(1);
    EXPECT_EQ(EMPTY, f.color(1, 1));
    EXPECT_EQ(0, f.height(1));

    f.removeTopPuyoFrom(1);
    EXPECT_EQ(EMPTY, f.color(1, 1));
    EXPECT_EQ(0, f.height(1));
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

TEST(FieldTest, TrackedFieldSimulation)
{
    Field f("400040"
            "456474"
            "445667"
            "556774");


    TrackedRensaInfo trackedRensaInfo;
    f.simulateAndTrack(trackedRensaInfo.rensaInfo, trackedRensaInfo.trackResult);

    EXPECT_EQ(5, trackedRensaInfo.rensaInfo.chains);
    EXPECT_EQ(1, trackedRensaInfo.trackResult.erasedAt(1, 2));
    EXPECT_EQ(2, trackedRensaInfo.trackResult.erasedAt(1, 1));
    EXPECT_EQ(3, trackedRensaInfo.trackResult.erasedAt(3, 3));
    EXPECT_EQ(4, trackedRensaInfo.trackResult.erasedAt(5, 3));
    EXPECT_EQ(5, trackedRensaInfo.trackResult.erasedAt(5, 4));
}

TEST(FieldTest, DropKumiPuyoExtreme)
{
    Field f1("http://www.inosendo.com/puyo/rensim/??110111550477450455745466754655664576755776666564777644555455666545775454444554");
    Field f2("http://www.inosendo.com/puyo/rensim/??110111550477451455745466754655664576755776666564777644555455666545775454444554");

    f1.dropKumiPuyoSafely(Decision(3, 0), KumiPuyo(RED, RED));
    f1.dropKumiPuyoSafely(Decision(3, 0), KumiPuyo(RED, RED));

    f2.dropKumiPuyoSafely(Decision(3, 0), KumiPuyo(RED, RED));
    f2.dropKumiPuyoSafely(Decision(3, 0), KumiPuyo(RED, RED));
}

TEST(FieldTest, FramesToDropNextWithoutChigiri)
{
    // TODO(mayah): We have to confirm this.
    Field f;

    EXPECT_EQ(Field::HEIGHT * FRAMES_DROP_1_LINE + FRAMES_AFTER_NO_CHIGIRI,
              f.framesToDropNext(Decision(3, 0)));
    EXPECT_EQ(Field::HEIGHT * FRAMES_DROP_1_LINE + FRAMES_AFTER_NO_CHIGIRI,
              f.framesToDropNext(Decision(3, 1)));
    EXPECT_EQ((Field::HEIGHT - 1) * FRAMES_DROP_1_LINE + FRAMES_AFTER_NO_CHIGIRI,
              f.framesToDropNext(Decision(3, 2)));
    EXPECT_EQ(Field::HEIGHT * FRAMES_DROP_1_LINE + FRAMES_AFTER_NO_CHIGIRI,
              f.framesToDropNext(Decision(3, 3)));
    EXPECT_EQ(Field::HEIGHT * FRAMES_DROP_1_LINE + FRAMES_HORIZONTAL_MOVE * 2 + FRAMES_AFTER_NO_CHIGIRI,
              f.framesToDropNext(Decision(1, 0)));
}

TEST(FieldTest, FramesToDropNextWithChigiri)
{
    Field f("004000"
            "005000"
            "006000"
            "007000");

    EXPECT_EQ((Field::HEIGHT - 4) * FRAMES_DROP_1_LINE + FRAMES_AFTER_CHIGIRI + FRAMES_CHIGIRI_1_LINE_1 + FRAMES_CHIGIRI_1_LINE_2 + 2 * FRAMES_CHIGIRI_1_LINE_3,
              f.framesToDropNext(Decision(3, 1)));
}
