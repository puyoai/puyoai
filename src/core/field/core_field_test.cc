#include "core/field/core_field.h"

#include <gtest/gtest.h>
#include <string>

#include "core/constant.h"
#include "core/decision.h"
#include "core/field/rensa_result.h"

using namespace std;

TEST(CoreFieldTest, Initial)
{
    CoreField f;

    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        for (int y = 1; y <= CoreField::HEIGHT; ++y) {
            EXPECT_EQ(PuyoColor::EMPTY, f.color(x, y)) << x << ' ' << y;
        }
    }
}

TEST(CoreFieldTest, Color)
{
    CoreField f("444000");

    EXPECT_EQ(PuyoColor::WALL, f.color(0, 1));
    EXPECT_EQ(PuyoColor::RED, f.color(1, 1));
    EXPECT_EQ(PuyoColor::RED, f.color(2, 1));
    EXPECT_EQ(PuyoColor::RED, f.color(3, 1));
    EXPECT_EQ(PuyoColor::EMPTY, f.color(4, 1));
    EXPECT_EQ(PuyoColor::EMPTY, f.color(5, 1));
    EXPECT_EQ(PuyoColor::EMPTY, f.color(6, 1));
    EXPECT_EQ(PuyoColor::WALL, f.color(7, 1));
}

TEST(CoreFieldTest, ForceDrop)
{
    CoreField f("544555"
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

    EXPECT_EQ(PuyoColor::RED, f.color(1, 1));
    EXPECT_EQ(PuyoColor::BLUE, f.color(1, 2));
    EXPECT_EQ(PuyoColor::EMPTY, f.color(1, 3));
    EXPECT_EQ(PuyoColor::EMPTY, f.color(1, 4));
}

void testUrl(string url, int expected_chains, int expected_score)
{
    CoreField f(url);
    RensaResult rensaResult = f.simulate();
    EXPECT_EQ(expected_chains, rensaResult.chains);
    EXPECT_EQ(expected_score, rensaResult.score);
}

TEST(CoreFieldTest, ChainAndScoreTest1)
{
    testUrl("http://www.inosendo.com/puyo/rensim/??444400", 1, 40);
}

TEST(CoreFieldTest, ChainAndScoreTest2)
{
    testUrl("http://www.inosendo.com/puyo/rensim/??5000005565444455", 2, 700);
}

TEST(CoreFieldTest, ChainAndScoreTest3)
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

TEST(CoreFieldTest, FramesTest) {
    {
        // 1 Rensa, no drop.
        CoreField f("444400");
        RensaResult rensaResult = f.simulate();
        EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_AFTER_NO_DROP + FRAMES_VANISH_ANIMATION,
                  rensaResult.frames);
    }
    {
        CoreField f("500000"
                "444400");
        RensaResult rensaResult = f.simulate();
        EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE + FRAMES_AFTER_DROP + FRAMES_VANISH_ANIMATION,
                  rensaResult.frames);
    }
    {
        CoreField f("500000"
                "400000"
                "444000");
        RensaResult rensaResult = f.simulate();
        EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP + FRAMES_VANISH_ANIMATION,
                  rensaResult.frames);
    }
    {
        CoreField f("500000"
                "450000"
                "444000");
        RensaResult rensaResult = f.simulate();
        EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP + FRAMES_VANISH_ANIMATION,
                  rensaResult.frames);
    }
    {
        CoreField f("500000"
                "455000"
                "444500");
        RensaResult rensaResult = f.simulate();
        EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP + FRAMES_VANISH_ANIMATION +
                  FRAMES_AFTER_VANISH + FRAMES_AFTER_NO_DROP + FRAMES_VANISH_ANIMATION,
                  rensaResult.frames);
    }
    {
        CoreField f("560000"
                "455000"
                "444500");
        RensaResult rensaResult = f.simulate();
        EXPECT_EQ(FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE * 2 + FRAMES_AFTER_DROP + FRAMES_VANISH_ANIMATION +
                  FRAMES_AFTER_VANISH + FRAMES_DROP_1_LINE + FRAMES_AFTER_DROP + FRAMES_VANISH_ANIMATION,
                  rensaResult.frames);
    }
}

TEST(CoreFieldTest, Height)
{
    CoreField f("004100"
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

TEST(CoreFieldTest, HeightShouldBeCopied)
{
    CoreField f("004100"
            "004040"
            "004707"
            "014040");

    CoreField g(f);

    EXPECT_EQ(0, g.height(1));
    EXPECT_EQ(1, g.height(2));
    EXPECT_EQ(4, g.height(3));
    EXPECT_EQ(4, g.height(4));
    EXPECT_EQ(3, g.height(5));
    EXPECT_EQ(2, g.height(6));
}

TEST(CoreFieldTest, HeightAfterSimulate)
{
    CoreField f("050005"
            "050055"
            "445644"
            "445644");

    f.simulate();

    EXPECT_EQ(0, f.height(1));
    EXPECT_EQ(0, f.height(2));
    EXPECT_EQ(0, f.height(3));
    EXPECT_EQ(2, f.height(4));
    EXPECT_EQ(1, f.height(5));
    EXPECT_EQ(2, f.height(6));
}

TEST(CoreFieldTest, HeightAfterSimulate2)
{
    CoreField f("450005"
            "445665"
            "556455");

    f.simulate();

    EXPECT_EQ(3, f.height(1));
    EXPECT_EQ(3, f.height(2));
    EXPECT_EQ(2, f.height(3));
    EXPECT_EQ(2, f.height(4));
    EXPECT_EQ(1, f.height(5));
    EXPECT_EQ(0, f.height(6));
}

TEST(CoreFieldTest, DropPuyoOn)
{
    CoreField f("050005"
            "050055"
            "445644"
            "445644");

    f.dropPuyoOn(1, PuyoColor::RED);

    EXPECT_EQ(PuyoColor::RED, f.color(1, 3));
    EXPECT_EQ(3, f.height(1));
}

TEST(CoreFieldTest, RemoveTopPuyoFrom)
{
    CoreField f("456756");

    f.removeTopPuyoFrom(1);
    EXPECT_EQ(PuyoColor::EMPTY, f.color(1, 1));
    EXPECT_EQ(0, f.height(1));

    f.removeTopPuyoFrom(1);
    EXPECT_EQ(PuyoColor::EMPTY, f.color(1, 1));
    EXPECT_EQ(0, f.height(1));
}

TEST(CoreFieldTest, TrackedCoreFieldSimulation)
{
    CoreField f("400040"
                "456474"
                "445667"
                "556774");


    RensaTrackResult trackResult;
    RensaResult basicRensaResult = f.simulateAndTrack(&trackResult);

    EXPECT_EQ(5, basicRensaResult.chains);
    EXPECT_EQ(1, trackResult.erasedAt(1, 2));
    EXPECT_EQ(2, trackResult.erasedAt(1, 1));
    EXPECT_EQ(3, trackResult.erasedAt(3, 3));
    EXPECT_EQ(4, trackResult.erasedAt(5, 3));
    EXPECT_EQ(5, trackResult.erasedAt(5, 4));
}

TEST(CoreFieldTest, FramesToDropNextWithoutChigiri)
{
    // TODO(mayah): We have to confirm this.
    CoreField f;

    EXPECT_EQ(CoreField::HEIGHT * FRAMES_DROP_1_LINE + FRAMES_AFTER_NO_CHIGIRI,
              f.framesToDropNext(Decision(3, 0)));
    EXPECT_EQ(CoreField::HEIGHT * FRAMES_DROP_1_LINE + FRAMES_AFTER_NO_CHIGIRI,
              f.framesToDropNext(Decision(3, 1)));
    EXPECT_EQ((CoreField::HEIGHT - 1) * FRAMES_DROP_1_LINE + FRAMES_AFTER_NO_CHIGIRI,
              f.framesToDropNext(Decision(3, 2)));
    EXPECT_EQ(CoreField::HEIGHT * FRAMES_DROP_1_LINE + FRAMES_AFTER_NO_CHIGIRI,
              f.framesToDropNext(Decision(3, 3)));
    EXPECT_EQ(CoreField::HEIGHT * FRAMES_DROP_1_LINE + FRAMES_HORIZONTAL_MOVE * 2 + FRAMES_AFTER_NO_CHIGIRI,
              f.framesToDropNext(Decision(1, 0)));
}

TEST(CoreFieldTest, FramesToDropNextWithChigiri)
{
    CoreField f("004000"
            "005000"
            "006000"
            "007000");

    EXPECT_EQ((CoreField::HEIGHT - 4) * FRAMES_DROP_1_LINE + FRAMES_AFTER_CHIGIRI + FRAMES_CHIGIRI_1_LINE_1 + FRAMES_CHIGIRI_1_LINE_2 + 2 * FRAMES_CHIGIRI_1_LINE_3,
              f.framesToDropNext(Decision(3, 1)));
}

TEST(CoreFieldTest, SimulateWithOjama)
{
    CoreField f(
        "ORRRRO"
        "OOOOOO");

    RensaResult rensaResult = f.simulate();
    EXPECT_EQ(40, rensaResult.score);
    EXPECT_EQ(1, rensaResult.chains);
}

TEST(CoreFieldTest, countConnectedPuyos)
{
    CoreField f(
        " YYY Y"
        "BBBOYO"
        "RRRGGG");

    EXPECT_EQ(3, f.countConnectedPuyos(1, 1));
    EXPECT_EQ(3, f.countConnectedPuyos(4, 1));
    EXPECT_EQ(3, f.countConnectedPuyos(1, 2));
    EXPECT_EQ(1, f.countConnectedPuyos(5, 2));
}

TEST(CoreFieldTest, countConnectedPuyosEmptyCase1)
{
    CoreField f;

    EXPECT_EQ(72, f.countConnectedPuyos(3, 12));
}

TEST(CoreFieldTest, countConnectedPuyosEmptyCase2)
{
    CoreField f(
        "    O " // 12
        "    O "
        "    O "
        "    O "
        "    O " // 8
        "    O "
        "    O "
        "    O "
        "    O " // 4
        "    O "
        "    O "
        "    O ");

    EXPECT_EQ(48, f.countConnectedPuyos(3, 12));
}

TEST(CoreFieldTest, isChigiriDecision1)
{
    CoreField cf;
    EXPECT_FALSE(cf.isChigiriDecision(Decision(3, 0)));
    EXPECT_FALSE(cf.isChigiriDecision(Decision(3, 1)));
    EXPECT_FALSE(cf.isChigiriDecision(Decision(3, 2)));
    EXPECT_FALSE(cf.isChigiriDecision(Decision(3, 3)));
}

TEST(CoreFieldTest, isChigiriDecision2)
{
    CoreField cf("  O   ");
    EXPECT_FALSE(cf.isChigiriDecision(Decision(3, 0)));
    EXPECT_TRUE(cf.isChigiriDecision(Decision(3, 1)));
    EXPECT_FALSE(cf.isChigiriDecision(Decision(3, 2)));
    EXPECT_TRUE(cf.isChigiriDecision(Decision(3, 3)));
}
