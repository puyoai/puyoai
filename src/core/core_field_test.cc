#include "core/core_field.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <string>

#include "core/decision.h"
#include "core/frame.h"
#include "core/position.h"
#include "core/rensa_result.h"

using namespace std;

TEST(CoreFieldTest, constructor)
{
    CoreField cf;

    for (int x = 1; x <= FieldConstant::WIDTH; ++x) {
        for (int y = 1; y <= FieldConstant::HEIGHT; ++y) {
            EXPECT_EQ(PuyoColor::EMPTY, cf.color(x, y)) << x << ' ' << y;
        }
    }

    for (int x = 0; x < FieldConstant::MAP_WIDTH; ++x) {
        EXPECT_EQ(0, cf.height(x));
    }
}

TEST(CoreFieldTest, color)
{
    CoreField cf("RRR   ");

    EXPECT_EQ(PuyoColor::WALL, cf.color(0, 1));
    EXPECT_EQ(PuyoColor::RED, cf.color(1, 1));
    EXPECT_EQ(PuyoColor::RED, cf.color(2, 1));
    EXPECT_EQ(PuyoColor::RED, cf.color(3, 1));
    EXPECT_EQ(PuyoColor::EMPTY, cf.color(4, 1));
    EXPECT_EQ(PuyoColor::EMPTY, cf.color(5, 1));
    EXPECT_EQ(PuyoColor::EMPTY, cf.color(6, 1));
    EXPECT_EQ(PuyoColor::WALL, cf.color(7, 1));
}

TEST(CoreFieldTest, countColorPuyos1)
{
    CoreField cf;

    EXPECT_EQ(0, cf.countColorPuyos());
}

TEST(CoreFieldTest, countColorPuyos2)
{
    CoreField cf(
        "RRBBYY"
        "OO&&OO"
        "RRBBYY");

    EXPECT_EQ(12, cf.countColorPuyos());
}

TEST(CoreFieldTest, countColorPuyos3)
{
    CoreField cf(
        "RR.RRR" // 14
        "RR.RRR" // 13
        "RR.RRR" // 12
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    // Don't count 14th row.
    EXPECT_EQ(10, cf.countColorPuyos());
}

TEST(CoreFieldTest, countColor)
{
    CoreField cf(
        "O....." // 14
        "O....."
        "O....." // 12
        "O....."
        "O....."
        "O....."
        "O....." // 8
        "O....."
        "O....Y"
        "OYBB.B"
        "OYYRBB" // 4
        "OBRR&&"
        "OBBYYY"
        "RRRGGG"
    );

    EXPECT_EQ(6, cf.countColor(PuyoColor::RED));
    EXPECT_EQ(8, cf.countColor(PuyoColor::BLUE));
    EXPECT_EQ(7, cf.countColor(PuyoColor::YELLOW));
    EXPECT_EQ(3, cf.countColor(PuyoColor::GREEN));
    EXPECT_EQ(12, cf.countColor(PuyoColor::OJAMA)); // don't contain 14th row.
    EXPECT_EQ(40, cf.countColor(PuyoColor::EMPTY));
    EXPECT_EQ(2, cf.countColor(PuyoColor::IRON));
}

TEST(CoreFieldTest, countUnreachableSpaces1)
{
    CoreField cf(
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    EXPECT_EQ(0, cf.countUnreachableSpaces());
}

TEST(CoreFieldTest, countUnreachableSpaces2)
{
    CoreField cf(
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

    EXPECT_EQ(12, cf.countUnreachableSpaces());
}

TEST(CoreFieldTest, countReachableSpaces_empty)
{
    const CoreField cf;
    EXPECT_EQ(72, cf.countReachableSpaces());
}

TEST(CoreFieldTest, countReachableSpaces1)
{
    const CoreField cf(
        ".O.O.." // 12
        ".OOO.."
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    EXPECT_EQ(1, cf.countReachableSpaces());
}

TEST(CoreFieldTest, countReachableSpaces2)
{
    const CoreField cf(
        ".O...." // 12
        ".OOO.."
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    EXPECT_EQ(6, cf.countReachableSpaces());
}

TEST(CoreFieldTest, ridgeHeight1)
{
    CoreField cf(
        "  O   "
        "  O   "
        "  O   "
        "  O  O"
        "OOOOOO");

    EXPECT_EQ(0, cf.ridgeHeight(1));
    EXPECT_EQ(0, cf.ridgeHeight(2));
    EXPECT_EQ(4, cf.ridgeHeight(3));
    EXPECT_EQ(0, cf.ridgeHeight(4));
    EXPECT_EQ(0, cf.ridgeHeight(5));
    EXPECT_EQ(0, cf.ridgeHeight(6));
}

TEST(CoreFieldTest, ridgeHeight2)
{
    CoreField cf(
        "  O   "
        "  O   "
        "  OO  "
        " OOO O"
        "OOOOOO");

    EXPECT_EQ(0, cf.ridgeHeight(1));
    EXPECT_EQ(0, cf.ridgeHeight(2));
    EXPECT_EQ(2, cf.ridgeHeight(3));
    EXPECT_EQ(0, cf.ridgeHeight(4));
    EXPECT_EQ(0, cf.ridgeHeight(5));
    EXPECT_EQ(0, cf.ridgeHeight(6));
}

TEST(CoreFieldTest, ridgeHeight3)
{
    CoreField cf(
        "O O   "
        "OOO   "
        "OOO   "
        "OOOO O"
        "OOOOOO");

    EXPECT_EQ(0, cf.ridgeHeight(1));
    EXPECT_EQ(0, cf.ridgeHeight(2));
    EXPECT_EQ(1, cf.ridgeHeight(3));
    EXPECT_EQ(0, cf.ridgeHeight(4));
    EXPECT_EQ(0, cf.ridgeHeight(5));
    EXPECT_EQ(0, cf.ridgeHeight(6));
}

TEST(CoreFieldTest, valleyDepth1)
{
    CoreField cf(
        "OO...."
        "OO.OOO"
        "OO.OOO"
        "OO.OOO"
        "OO.OOO"
        "OOOOOO");

    EXPECT_EQ(0, cf.valleyDepth(1));
    EXPECT_EQ(0, cf.valleyDepth(2));
    EXPECT_EQ(4, cf.valleyDepth(3));
    EXPECT_EQ(0, cf.valleyDepth(4));
    EXPECT_EQ(0, cf.valleyDepth(5));
    EXPECT_EQ(0, cf.valleyDepth(6));
}

TEST(CoreFieldTest, valleyDepth2)
{
    CoreField cf(
        ".O...."
        ".O.O.."
        "OO.O.."
        "OO.O.O"
        "OO.OOO"
        "OOOOOO");

    EXPECT_EQ(2, cf.valleyDepth(1));
    EXPECT_EQ(0, cf.valleyDepth(2));
    EXPECT_EQ(4, cf.valleyDepth(3));
    EXPECT_EQ(0, cf.valleyDepth(4));
    EXPECT_EQ(1, cf.valleyDepth(5));
    EXPECT_EQ(0, cf.valleyDepth(6));
}

TEST(CoreFieldTest, simulate1)
{
    CoreField cf("RRRR..");

    RensaResult rensaResult = cf.simulate();
    EXPECT_EQ(1, rensaResult.chains);
    EXPECT_EQ(40, rensaResult.score);
    EXPECT_EQ(FRAMES_VANISH_ANIMATION, rensaResult.frames);
}

TEST(CoreFieldTest, simulate2)
{
    CoreField cf(
        "..B..."
        "..BBYB"
        "RRRRBB");

    RensaResult rensaResult = cf.simulate();
    EXPECT_EQ(2, rensaResult.chains);
    EXPECT_EQ(700, rensaResult.score);

    int frames = 0;
    frames += FRAMES_VANISH_ANIMATION + FRAMES_TO_DROP_FAST[1] + FRAMES_GROUNDING;
    frames += FRAMES_VANISH_ANIMATION + FRAMES_TO_DROP_FAST[1] + FRAMES_GROUNDING;
    EXPECT_EQ(frames, rensaResult.frames);
}

void testUrl(string url, int expected_chains, int expected_score)
{
    CoreField f(url);
    RensaResult rensaResult = f.simulate();
    EXPECT_EQ(expected_chains, rensaResult.chains);
    EXPECT_EQ(expected_score, rensaResult.score);
}

TEST(CoreFieldTest, ChainAndScoreTest3)
{
    testUrl("050745574464446676456474656476657564547564747676466766747674757644657575475755", 19, 175080);
    testUrl("500467767675744454754657447767667644674545455767477644457474656455446775455646", 19, 175080);
    testUrl("000550050455045451045745074745074645067674067674056567056567515167444416555155", 2, 38540);
    testUrl("050550040455075451075745064745064645067674057674747574776567675156644415555155", 3, 43260);
    testUrl("000550040455075451775745464745464645467674457674147574776567675156644415555155", 4, 50140);
    testUrl("745550576455666451175745564745564745567674157674747574776566615156644415555155", 5, 68700);
    testUrl("444411114141414114114111414144411114414111114414411114441114111141444141111141", 4, 4840);
    testUrl("545544544454454545454545454545545454445544554455454545545454554544445455455445", 9, 49950);
    testUrl("444446544611446164564441546166565615454551441444111111111111111111111111111111", 9, 32760);
    testUrl("667547466555661677471475451447461666661547457477556446776555744646476466744555", 18, 155980);
    testUrl("444044144414114144411411414144141414414141441414114411441144414141141414144144", 11, 47080);
    testUrl("000000444444444444444444444444444444444444444444444444444444444444444444444444", 1, 7200);
}

TEST(CoreFieldTest, FramesTest) {
    {
        // 1 Rensa, no drop.
        CoreField f("RRRR..");
        RensaResult rensaResult = f.simulate();
        EXPECT_EQ(FRAMES_VANISH_ANIMATION, rensaResult.frames);
    }
    {
        CoreField f("Y....."
                    "RRRR..");
        RensaResult rensaResult = f.simulate();
        EXPECT_EQ(FRAMES_VANISH_ANIMATION + FRAMES_TO_DROP_FAST[1] + FRAMES_GROUNDING, rensaResult.frames);
    }
    {
        CoreField f("Y....."
                    "R....."
                    "RRR...");
        RensaResult rensaResult = f.simulate();
        EXPECT_EQ(FRAMES_VANISH_ANIMATION + FRAMES_TO_DROP_FAST[2] + FRAMES_GROUNDING, rensaResult.frames);
    }
    {
        CoreField f("Y....."
                    "RY...."
                    "RRR...");
        RensaResult rensaResult = f.simulate();
        EXPECT_EQ(FRAMES_VANISH_ANIMATION + FRAMES_TO_DROP_FAST[2] + FRAMES_GROUNDING, rensaResult.frames);
    }
    {
        CoreField f("Y....."
                    "RYY..."
                    "RRRY..");
        RensaResult rensaResult = f.simulate();
        EXPECT_EQ(FRAMES_VANISH_ANIMATION + FRAMES_TO_DROP_FAST[2] + FRAMES_GROUNDING +
                  FRAMES_VANISH_ANIMATION,
                  rensaResult.frames);
    }
    {
        CoreField f("YB...."
                    "RYY..."
                    "RRRY..");
        RensaResult rensaResult = f.simulate();
        EXPECT_EQ(FRAMES_VANISH_ANIMATION + FRAMES_TO_DROP_FAST[2] + FRAMES_GROUNDING +
                  FRAMES_VANISH_ANIMATION + FRAMES_TO_DROP_FAST[1] + FRAMES_GROUNDING,
                  rensaResult.frames);
    }
}

TEST(CoreFieldTest, vanishWithIron)
{
    CoreField cf(
        ".OO.&&"
        "RRRR&&");

    CoreField expected(
        "....&&"
        "....&&");

    cf.simulate();

    EXPECT_EQ(expected, cf) << cf.toDebugString();
}

TEST(CoreFieldTest, height)
{
    CoreField f("..O..."
                "..O.O."
                "..OOOO"
                ".OOOOO");

    EXPECT_EQ(0, f.height(1));
    EXPECT_EQ(1, f.height(2));
    EXPECT_EQ(4, f.height(3));
    EXPECT_EQ(2, f.height(4));
    EXPECT_EQ(3, f.height(5));
    EXPECT_EQ(2, f.height(6));
}

TEST(CoreFieldTest, heightShouldBeCopied)
{
    CoreField f("..O..."
                "..O.O."
                "..OOOO"
                ".OOOOO");

    CoreField g(f);

    EXPECT_EQ(0, g.height(1));
    EXPECT_EQ(1, g.height(2));
    EXPECT_EQ(4, g.height(3));
    EXPECT_EQ(2, g.height(4));
    EXPECT_EQ(3, g.height(5));
    EXPECT_EQ(2, g.height(6));
}

TEST(CoreFieldTest, HeightAfterSimulate)
{
    CoreField f(".B...B"
                ".B..BB"
                "RRBYRR"
                "RRBYRR");

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
    CoreField f("RG...Y"
                "RRGBBY"
                "GGBRYY");

    f.simulate();

    EXPECT_EQ(3, f.height(1));
    EXPECT_EQ(3, f.height(2));
    EXPECT_EQ(2, f.height(3));
    EXPECT_EQ(2, f.height(4));
    EXPECT_EQ(1, f.height(5));
    EXPECT_EQ(0, f.height(6));
}

TEST(CoreFieldTest, quick)
{
    CoreField f("GGGG  "
                "RRRR  ");

    RensaResult r = f.simulate();
    EXPECT_TRUE(r.quick);
}

TEST(CoreFieldTest, notQuick)
{
    CoreField f("  YY  "
                "GGGG  "
                "RRRR  ");

    RensaResult r = f.simulate();
    EXPECT_FALSE(r.quick);
}

TEST(CoreFieldTest, fallOjama1)
{
    CoreField cf;
    CoreField expected(
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    int framesOjamaDropping = cf.fallOjama(3);
    int expectedFrames = FRAMES_TO_DROP[12] + framesGroundingOjama(18);

    EXPECT_EQ(expected, cf) << cf.toDebugString();
    EXPECT_EQ(expectedFrames, framesOjamaDropping);
}

TEST(CoreFieldTest, fallOjama2)
{
    CoreField cf(
        ".....Y" // 14 (some puyo is in the air).
        "......"
        "......"
        "O....O"
        "OO.OOO" // 10
        "OO.OOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");
    CoreField expected(
        ".....Y"
        "OO.OOO" // 13
        "OO.OOO" // 12
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    // Ojama won't drop on 14th line.
    int framesOjamaDropping = cf.fallOjama(3);
    int expectedFrames = FRAMES_TO_DROP[4] + framesGroundingOjama(18);

    EXPECT_EQ(expected, cf) << cf.toDebugString();
    EXPECT_EQ(expectedFrames, framesOjamaDropping);
}

TEST(CoreFieldTest, dropPuyoOn)
{
    CoreField f(".O...." // 12
                ".O...."
                ".O...."
                ".O...."
                ".O...." // 8
                ".O...."
                ".O...."
                ".O...."
                ".O...." // 4
                ".O...."
                ".O...."
                ".O....");

    EXPECT_TRUE(f.dropPuyoOn(2, PuyoColor::RED));
    EXPECT_EQ(PuyoColor::RED, f.color(2, 13));
    EXPECT_EQ(13, f.height(2));

    EXPECT_FALSE(f.dropPuyoOn(2, PuyoColor::RED));
    EXPECT_EQ(13, f.height(2));
}

TEST(CoreFieldTest, dropPuyoOnWithMaxHeight)
{
    CoreField f("..R..."
                "..R..."
                "..R...");

    EXPECT_FALSE(f.dropPuyoOnWithMaxHeight(3, PuyoColor::GREEN, 3));
    EXPECT_EQ(3, f.height(3));

    EXPECT_TRUE(f.dropPuyoOnWithMaxHeight(3, PuyoColor::GREEN, 4));
    EXPECT_EQ(4, f.height(3));
    EXPECT_EQ(PuyoColor::GREEN, f.color(3, 4));
}

TEST(CoreFieldTest, dropPuyoOnWithMaxHeightEdgeCase)
{
    CoreField cf("OO.OOO"
                 "O....." // 13
                 "OOOOOO" // 12
                 "OOOOOO"
                 "OOOOOO"
                 "OOOOOO"
                 "OOOOOO" // 8
                 "OOOOOO"
                 "OOOOOO"
                 "OOOOOO"
                 "OOOOOO" // 4
                 "OOOOOO"
                 "OOOOOO"
                 "OOOOOO");

    EXPECT_FALSE(cf.dropPuyoOnWithMaxHeight(1, PuyoColor::RED, 14));
}

TEST(CoreFieldTest, removePuyoFrom)
{
    CoreField cf(
        "...OOO"
        "OOOOOO");

    cf.removePuyoFrom(1);
    cf.removePuyoFrom(2);
    cf.removePuyoFrom(3);
    EXPECT_EQ(PuyoColor::EMPTY, cf.color(1, 1));
    EXPECT_EQ(PuyoColor::EMPTY, cf.color(2, 1));
    EXPECT_EQ(PuyoColor::EMPTY, cf.color(3, 1));
    EXPECT_EQ(0, cf.height(1));
    EXPECT_EQ(0, cf.height(2));
    EXPECT_EQ(0, cf.height(3));
}

TEST(CoreFieldTest, framesToDropNextWithoutChigiri)
{
    // TODO(mayah): We have to confirm this.
    CoreField f;

    EXPECT_EQ(FRAMES_TO_DROP_FAST[FieldConstant::HEIGHT] + FRAMES_GROUNDING,
              f.framesToDropNext(Decision(3, 0)));
    EXPECT_EQ(FRAMES_TO_DROP_FAST[FieldConstant::HEIGHT] + FRAMES_GROUNDING,
              f.framesToDropNext(Decision(3, 1)));
    EXPECT_EQ(FRAMES_TO_DROP_FAST[FieldConstant::HEIGHT - 1] + FRAMES_GROUNDING,
              f.framesToDropNext(Decision(3, 2)));
    EXPECT_EQ(FRAMES_TO_DROP_FAST[FieldConstant::HEIGHT] + FRAMES_GROUNDING,
              f.framesToDropNext(Decision(3, 3)));
    EXPECT_EQ(FRAMES_TO_MOVE_HORIZONTALLY[2] + FRAMES_TO_DROP_FAST[FieldConstant::HEIGHT] + FRAMES_GROUNDING,
              f.framesToDropNext(Decision(1, 0)));
}

TEST(CoreFieldTest, framesToDropNextWithChigiri)
{
    CoreField f("..O..."
                "..O..."
                "..O..."
                "..O...");

    EXPECT_EQ(FRAMES_TO_DROP_FAST[FieldConstant::HEIGHT - 4] + FRAMES_GROUNDING +
              FRAMES_TO_DROP[4] + FRAMES_GROUNDING,
              f.framesToDropNext(Decision(3, 1)));
}

TEST(CoreFieldTest, framesToDropNextOn13thRow)
{
    CoreField f(
        "OO OOO" // 12
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    // We cannot put with Decision(4, 2).

    EXPECT_EQ(6 + FRAMES_TO_MOVE_HORIZONTALLY[1] + FRAMES_GROUNDING, f.framesToDropNext(Decision(4, 0)));
    EXPECT_EQ(6 + FRAMES_TO_MOVE_HORIZONTALLY[1] + FRAMES_GROUNDING, f.framesToDropNext(Decision(4, 1)));
    EXPECT_EQ(6 + FRAMES_TO_MOVE_HORIZONTALLY[1] + FRAMES_GROUNDING + FRAMES_TO_DROP[1] + FRAMES_GROUNDING,
              f.framesToDropNext(Decision(4, 3)));
}

TEST(CoreFieldTest, SimulateWithOjama)
{
    CoreField f("ORRRRO"
                "OOOOOO");

    RensaResult rensaResult = f.simulate();
    EXPECT_EQ(40, rensaResult.score);
    EXPECT_EQ(1, rensaResult.chains);
}

TEST(CoreFieldTest, countConnectedPuyos)
{
    CoreField f(" YYY Y"
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

TEST(CoreFieldTest, countConnectedPuyosInvisibleCase)
{
    CoreField f(
        "B     "  // 13
        "BY    "  // 12
        "BB    "
        "YY    "
        "YB R  "
        "GYBY  "  //  8
        "RGYG  "
        "BGGRR "
        "BBRGR "
        "RYRYYY"  //  4
        "RRBGRR"
        "YYYBGG"
        "RBBRGR");

    EXPECT_EQ(3, f.countConnectedPuyos(1, 11));
    EXPECT_EQ(3, f.countConnectedPuyos(1, 12));
    EXPECT_EQ(0, f.countConnectedPuyos(1, 13));
}

TEST(CoreFieldTest, countConnectedPuyosMax4)
{
    CoreField f(" YYY Y"
                "BBBOYO"
                "RRRGGG");

    EXPECT_EQ(3, f.countConnectedPuyosMax4(1, 1));
    EXPECT_EQ(3, f.countConnectedPuyosMax4(4, 1));
    EXPECT_EQ(3, f.countConnectedPuyosMax4(1, 2));
    EXPECT_EQ(1, f.countConnectedPuyosMax4(5, 2));
}

TEST(CoreFieldTest, countConnectedPuyosMax4EdgeCase)
{
    CoreField f(
      "YYYGGG" // 13
      "YYYGGG" // 12
      "OOOOOO"
      "OOOOOO"
      "OOOOOO"
      "OOOOOO" // 8
      "OOOOOO"
      "OOOOOO"
      "OOOOOO"
      "OOOOOO" // 4
      "OOOOOO"
      "OOOOOO"
      "OOOOOO");

    for (int x = 1; x <= FieldConstant::WIDTH; ++x) {
        EXPECT_EQ(3, f.countConnectedPuyosMax4(x, FieldConstant::HEIGHT));
        EXPECT_EQ(0, f.countConnectedPuyosMax4(x, FieldConstant::HEIGHT + 1));
    }
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

TEST(CoreFieldTest, rensaWillOccurWhenLastDecisionIs)
{
  CoreField cf(
               "     G"  // 13
               "    GG"  // 12
               "    GY"
               "BGY BG"
               "YYGBBG"
               "GRRYRG"  //  8
               "BBBYRR"
               "YYYRBG"
               "BGGBGG"
               "BBGBRB"  //  4
               "GYYBRG"
               "BBBRBY"
               "YRRGGG");
  EXPECT_FALSE(cf.rensaWillOccurWhenLastDecisionIs(Decision(5, 1)));
}

TEST(CoreFieldTest, rensaWillOccur)
{
    CoreField cf1(
        "YYYY  ");
    CoreField cf2(
        " YYY  ");
    CoreField cf3(
        " GGG  "
        "YYYY  ");
    CoreField cf4(
        " GGG  "
        "YYYYY ");
    CoreField cf5(
        "OOOO  ");

    EXPECT_TRUE(cf1.rensaWillOccur());
    EXPECT_FALSE(cf2.rensaWillOccur());
    EXPECT_TRUE(cf3.rensaWillOccur());
    EXPECT_TRUE(cf4.rensaWillOccur());
    EXPECT_FALSE(cf5.rensaWillOccur());
}

TEST(CoreFieldTest, vanishDrop)
{
    CoreField cf(
        "..BB.."
        "RRRRBB");
    CoreField::SimulationContext context(2);

    RensaStepResult stepResult = cf.vanishDrop(&context);

    CoreField expected(
        "..BBBB");

    EXPECT_EQ(expected, cf);
    EXPECT_EQ(40 * 8, stepResult.score);
    EXPECT_EQ(FRAMES_GROUNDING + FRAMES_VANISH_ANIMATION + FRAMES_TO_DROP_FAST[1], stepResult.frames);
    EXPECT_FALSE(stepResult.quick);
}

TEST(CoreFieldTest, vanishDrop_quick)
{
    CoreField cf(
        "......"
        "RRRRBB");
    CoreField::SimulationContext context(2);

    RensaStepResult stepResult = cf.vanishDrop(&context);

    CoreField expected(
        "....BB");

    EXPECT_EQ(expected, cf);
    EXPECT_EQ(40 * 8, stepResult.score);
    EXPECT_EQ(FRAMES_VANISH_ANIMATION, stepResult.frames);
    EXPECT_TRUE(stepResult.quick);
}

TEST(CoreFieldTest, erasingPuyoPositions1)
{
    CoreField cf(
        "..BB.."
        "RRRRBB");

    vector<Position> positions = cf.erasingPuyoPositions();
    std::sort(positions.begin(), positions.end());

    vector<Position> expected = {
        Position(1, 1),
        Position(2, 1),
        Position(3, 1),
        Position(4, 1),
    };

    EXPECT_EQ(expected, positions);
}

TEST(CoreFieldTest, erasingPuyoPositions2)
{
    CoreField cf(
        ".OBBOO"
        "RRRRBB");

    vector<Position> positions = cf.erasingPuyoPositions();
    std::sort(positions.begin(), positions.end());

    vector<Position> expected = {
        Position(1, 1),
        Position(2, 1),
        Position(2, 2),
        Position(3, 1),
        Position(4, 1),
    };

    EXPECT_EQ(expected, positions);
}
