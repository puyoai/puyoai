#include "core/ctrl.h"

#include <gtest/gtest.h>
#include <vector>

#include "core/decision.h"
#include "core/kumipuyo.h"
#include "core/plain_field.h"

using namespace std;

TEST(CtrlTest, moveKumipuyoWithOnlyArrowKey)
{
    PlainField f;
    MovingKumipuyoState mks(KumipuyoPos(3, 12, 0));
    bool downAccepted = false;

    Ctrl::moveKumipuyo(f, KeySet(Key::KEY_RIGHT), &mks, &downAccepted);
    EXPECT_EQ(KumipuyoPos(4, 12, 0), mks.pos);
    EXPECT_EQ(0, mks.restFramesToAcceptQuickTurn);
    EXPECT_EQ(FRAMES_FREE_FALL - 1, mks.restFramesForFreefall);
    EXPECT_FALSE(mks.grounded);

    Ctrl::moveKumipuyo(f, KeySet(Key::KEY_LEFT), &mks, &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 12, 0), mks.pos);
    EXPECT_EQ(0, mks.restFramesToAcceptQuickTurn);
    EXPECT_EQ(FRAMES_FREE_FALL - 2, mks.restFramesForFreefall);
    EXPECT_FALSE(mks.grounded);

    Ctrl::moveKumipuyo(f, KeySet(Key::KEY_DOWN), &mks, &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 12, 0), mks.pos);
    EXPECT_EQ(0, mks.restFramesToAcceptQuickTurn);
    EXPECT_EQ(0, mks.restFramesForFreefall);
    EXPECT_FALSE(mks.grounded);
    EXPECT_TRUE(downAccepted);

    downAccepted = false;
    Ctrl::moveKumipuyo(f, KeySet(Key::KEY_DOWN), &mks, &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 11, 0), mks.pos);
    EXPECT_EQ(0, mks.restFramesToAcceptQuickTurn);
    EXPECT_EQ(0, mks.restFramesForFreefall);
    EXPECT_FALSE(mks.grounded);
    EXPECT_TRUE(downAccepted);

    mks.pos = KumipuyoPos(3, 1, 0);
    mks.restFramesForFreefall = FRAMES_FREE_FALL;

    downAccepted = false;
    Ctrl::moveKumipuyo(f, KeySet(Key::KEY_DOWN), &mks, &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 1, 0), mks.pos);
    EXPECT_EQ(0, mks.restFramesToAcceptQuickTurn);
    EXPECT_EQ(0, mks.restFramesForFreefall);
    EXPECT_FALSE(mks.grounded);
    EXPECT_TRUE(downAccepted);

    downAccepted = false;
    Ctrl::moveKumipuyo(f, KeySet(Key::KEY_DOWN), &mks, &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 1, 0), mks.pos);
    EXPECT_EQ(0, mks.restFramesToAcceptQuickTurn);
    EXPECT_EQ(0, mks.restFramesForFreefall);
    EXPECT_TRUE(mks.grounded);
    EXPECT_TRUE(downAccepted);
}

TEST(CtrlTest, moveKumipuyoFreefall)
{
    PlainField f;
    MovingKumipuyoState mks(KumipuyoPos(3, 12, 0));
    mks.restFramesForFreefall = 2;
    bool downAccepted = false;

    Ctrl::moveKumipuyo(f, KeySet(), &mks, &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 12, 0), mks.pos);
    EXPECT_EQ(0, mks.restFramesToAcceptQuickTurn);
    EXPECT_EQ(1, mks.restFramesForFreefall);
    EXPECT_FALSE(mks.grounded);
    EXPECT_FALSE(downAccepted);

    Ctrl::moveKumipuyo(f, KeySet(), &mks, &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 11, 0), mks.pos);
    EXPECT_EQ(0, mks.restFramesToAcceptQuickTurn);
    EXPECT_EQ(FRAMES_FREE_FALL, mks.restFramesForFreefall);
    EXPECT_FALSE(mks.grounded);
    EXPECT_FALSE(downAccepted);

    mks.pos = KumipuyoPos(3, 1, 0);
    mks.restFramesForFreefall = 2;

    Ctrl::moveKumipuyo(f, KeySet(), &mks, &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 1, 0), mks.pos);
    EXPECT_EQ(0, mks.restFramesToAcceptQuickTurn);
    EXPECT_EQ(1, mks.restFramesForFreefall);
    EXPECT_FALSE(mks.grounded);
    EXPECT_FALSE(downAccepted);

    Ctrl::moveKumipuyo(f, KeySet(), &mks, &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 1, 0), mks.pos);
    EXPECT_EQ(0, mks.restFramesToAcceptQuickTurn);
    EXPECT_EQ(FRAMES_FREE_FALL, mks.restFramesForFreefall);
    EXPECT_TRUE(mks.grounded);
    EXPECT_FALSE(downAccepted);
}

TEST(CtrlTest, moveKumipuyoMultipleKeys)
{
    PlainField f;
    MovingKumipuyoState mks(KumipuyoPos(1, 12, 0));
    mks.restFramesForFreefall = FRAMES_FREE_FALL;
    bool downAccepted = false;

    Ctrl::moveKumipuyo(f, KeySet(Key::KEY_RIGHT, Key::KEY_LEFT_TURN), &mks, &downAccepted);
    EXPECT_EQ(KumipuyoPos(2, 12, 3), mks.pos);
    EXPECT_EQ(0, mks.restFramesToAcceptQuickTurn);
    EXPECT_EQ(FRAMES_FREE_FALL - 1, mks.restFramesForFreefall);
    EXPECT_FALSE(mks.grounded);
    EXPECT_FALSE(downAccepted);
}

TEST(CtrlTest, isReachable1)
{
    PlainField f(
        "      "
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r <= 3; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;

            EXPECT_TRUE(Ctrl::isReachable(f, d));
        }
    }
}

TEST(CtrlTest, isReachable2)
{
    PlainField f(
        " O O  "
        " O O  "
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r <= 3; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;

            if (x == 3 && (r == 0 || r == 2))
                EXPECT_TRUE(Ctrl::isReachable(f, d)) << d.toString();
            else
                EXPECT_FALSE(Ctrl::isReachable(f, d)) << d.toString();
        }
    }
}

TEST(CtrlTest, isReachableOnline)
{
    // need quick turn to go over a wall
    PlainField f("040400"
                 "040400"
                 "040400"
                 "040400"
                 "040400"
                 "040400"
                 "040400"
                 "040400"
                 "040400"
                 "040400"
                 "040400"
                 "040400");
    KumipuyoPos k0(3, 12, 0);  // initial position
    KumipuyoPos k1(3, 11, 0);
    for (int x = 1; x <= 6; x++) {
        KumipuyoPos goal(x, 1, 0);
        EXPECT_TRUE(Ctrl::isReachableOnline(f, goal, k0));
    }
    EXPECT_FALSE(Ctrl::isReachableOnline(f, KumipuyoPos(1, 1, 2), k1));
    EXPECT_FALSE(Ctrl::isReachableOnline(f, KumipuyoPos(5, 1, 0), k1));
}

TEST(CtrlTest, isReachableCannotClimbTwoBlocks)
{
    PlainField f("040040"
                 "050050");
    /*
      3  ......
      2  .@A.@.
      1  .@..@.  A: jiku-puyo
    */
    KumipuyoPos k(3, 2, 0);
    EXPECT_FALSE(Ctrl::isReachableOnline(f, KumipuyoPos(1, 1, 0), k));
    EXPECT_FALSE(Ctrl::isReachableOnline(f, KumipuyoPos(2, 3, 0), k));
    EXPECT_FALSE(Ctrl::isReachableOnline(f, KumipuyoPos(5, 3, 0), k));
    EXPECT_FALSE(Ctrl::isReachableOnline(f, KumipuyoPos(6, 1, 0), k));
}

TEST(CtrlTest, climbStairsRight)
{
    PlainField f("000004"
                 "000040"
                 "000400"
                 "004000"
                 "040000");
    /*
      6  ......
      5  .....@
      4  ....@.
      3  ...@..
      2  B.@...
      1  A@....  A: jiku-puyo
    */
    KumipuyoPos k0(1, 1, 0);
    for (int x = 1; x <= 6; x++) {
        KumipuyoPos goal(x, x, 0);
        EXPECT_TRUE(Ctrl::isReachableOnline(f, goal, k0));
        KeySetSeq ret;
        Ctrl::getControlOnline(f, goal, k0, &ret);
        if (x == 4) {
            // TODO(yamaguchi): update this test when getControlOnline is updated.
            EXPECT_EQ("B,B,A,A,B,B,A,>,A,B,B,A,>,A,v", ret.toString());
        }
    }
}

TEST(CtrlTest, climbStairsLeft)
{
    PlainField f("400000"
                 "040000"
                 "004000"
                 "000400"
                 "000040"
                 "000040");
    /*
      6  @.....
      5  .@....
      4  ..@...
      3  ...@.B
      2  ....@A
      1  ....@.  A: jiku-puyo
    */
    KumipuyoPos k0(6, 2, 0);
    for (int x = 1; x <= 6; x++) {
        KumipuyoPos goal(x, 8 - x, 0);
        EXPECT_TRUE(Ctrl::isReachableOnline(f, goal, k0));
        KeySetSeq ret;
        Ctrl::getControlOnline(f, goal, k0, &ret);
        if (x == 4) {
            // TODO(yamaguchi): update this test when getControlOnline is updated.
            EXPECT_EQ("A,A,B,B,A,A,B,<,B,v", ret.toString());
        }
    }
}

TEST(CtrlTest, simpleMove)
{
    PlainField f;
    KeySetSeq ret;
    Ctrl::getControlOnline(f, KumipuyoPos(3, 1, 0), KumipuyoPos::InitialPos(), &ret);
    EXPECT_EQ("v", ret.toString());
    Ctrl::getControlOnline(f, KumipuyoPos(2, 1, 0), KumipuyoPos::InitialPos(), &ret);
    EXPECT_EQ("<,v", ret.toString());
    Ctrl::getControlOnline(f, KumipuyoPos(5, 1, 0), KumipuyoPos::InitialPos(), &ret);
    EXPECT_EQ(">,>,v", ret.toString());
    Ctrl::getControlOnline(f, KumipuyoPos(6, 1, 2), KumipuyoPos::InitialPos(), &ret);
    EXPECT_EQ(">,>,>,B,B,v", ret.toString());
    Ctrl::getControlOnline(f, KumipuyoPos(1, 1, 2), KumipuyoPos::InitialPos(), &ret);
    EXPECT_EQ("<,<,A,A,v", ret.toString());
    Ctrl::getControlOnline(f, KumipuyoPos(6, 1, 0), KumipuyoPos(1, 1, 2), &ret);
    EXPECT_EQ("B,B,>,>,>,>,>,v", ret.toString());
}

TEST(CtrlTest, subpuyoIsHigher)
{
    PlainField f("000040");
    KeySetSeq ret;
    Ctrl::getControlOnline(f, KumipuyoPos(4, 2, 1), KumipuyoPos(3, 1, 0), &ret);
    EXPECT_EQ(">,B,B,B,v", ret.toString());
}

TEST(CtrlTest, wallAboveScreen)
{
    /*
      14 ......
      13 .@B@..
      12 .@A@..
      11 .@@@..
    */
    PlainField f("040400"
                 "040400"
                 "044400"
                 "000000"
                 "000000"
                 "000000"
                 "000000"
                 "000000"
                 "000000"
                 "000000"
                 "000000"
                 "000000"
                 "000000");
    EXPECT_FALSE(Ctrl::isReachableOnline(f, KumipuyoPos(1, 1, 0), KumipuyoPos::InitialPos()));
    EXPECT_FALSE(Ctrl::isReachableOnline(f, KumipuyoPos(5, 1, 0), KumipuyoPos::InitialPos()));
}

TEST(CtrlTest, pivotCannotClimbUpTo14)
{
    /*
      14 ......
      13 @.B.@@
      12 .@A@..
      11 ..@...
    */
    PlainField f("400044"
                 "040400"
                 "004000"
                 "000000"
                 "000000"
                 "000000"
                 "000000"
                 "000000"
                 "000000"
                 "000000"
                 "000000"
                 "000000"
                 "000000");
    EXPECT_FALSE(Ctrl::isReachableOnline(f, KumipuyoPos(1, 1, 0), KumipuyoPos::InitialPos()));
    EXPECT_FALSE(Ctrl::isReachableOnline(f, KumipuyoPos(5, 1, 0), KumipuyoPos::InitialPos()));
}

TEST(CtrlTest, isReachable)
{
    // need quick turn to go over a wall
    PlainField f("000000"
                 "040400"
                 "040400"
                 "040400"
                 "040400"
                 "040400"
                 "040400"
                 "040400"
                 "040400"
                 "040400"
                 "040400"
                 "040400"
                 "040400");
    for (int x = 1; x <= 6; x++) {
        EXPECT_TRUE(Ctrl::isReachable(f, Decision(x, 0)));
        if (x != 3 && x != 6) {
            EXPECT_TRUE(Ctrl::isReachable(f, Decision(x, 1)));
        }
        EXPECT_TRUE(Ctrl::isReachable(f, Decision(x, 2)));
        if (x != 1 &&  x != 3) {
            EXPECT_TRUE(Ctrl::isReachable(f, Decision(x, 3)));
        }
    }
}
