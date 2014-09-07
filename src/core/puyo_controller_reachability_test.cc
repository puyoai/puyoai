#include "core/puyo_controller.h"

#include <gtest/gtest.h>
#include <vector>

#include "core/decision.h"
#include "core/kumipuyo.h"
#include "core/plain_field.h"

using namespace std;

TEST(PuyoControllerReachabilityTest, reachableOnEmptyField)
{
    PlainField f;

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r <= 3; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;

            EXPECT_TRUE(PuyoController::isReachable(f, d));
        }
    }
}

TEST(PuyoControllerReachabilityTest, reachableOnHigher)
{
    PlainField f(
        "      " // 12
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

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r <= 3; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;

            EXPECT_TRUE(PuyoController::isReachable(f, d));
        }
    }
}

TEST(PuyoControllerReachabilityTest, reachableOn13thline)
{
    PlainField f(
        "O    O" // 12
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

    EXPECT_TRUE(PuyoController::isReachable(f, Decision(1, 0)));
    EXPECT_TRUE(PuyoController::isReachable(f, Decision(1, 1)));
    EXPECT_FALSE(PuyoController::isReachable(f, Decision(1, 2)));
    EXPECT_TRUE(PuyoController::isReachable(f, Decision(2, 3)));

    EXPECT_TRUE(PuyoController::isReachable(f, Decision(6, 0)));
    EXPECT_TRUE(PuyoController::isReachable(f, Decision(6, 3)));
    EXPECT_FALSE(PuyoController::isReachable(f, Decision(6, 2)));
    EXPECT_TRUE(PuyoController::isReachable(f, Decision(5, 1)));
}

TEST(PuyoControllerReachabilityTest, beyondWall)
{
    // need quick turn to go over a wall
    PlainField f(" O O  " // 12
                 " O O  "
                 " O O  "
                 " O O  "
                 " O O  " // 8
                 " O O  "
                 " O O  "
                 " O O  "
                 " O O  " // 4
                 " O O  "
                 " O O  "
                 " O O  ");

    EXPECT_TRUE(PuyoController::isReachable(f, Decision(1, 0)));
    EXPECT_TRUE(PuyoController::isReachable(f, Decision(2, 0)));
    EXPECT_TRUE(PuyoController::isReachable(f, Decision(3, 0)));
    EXPECT_TRUE(PuyoController::isReachable(f, Decision(4, 0)));
    EXPECT_TRUE(PuyoController::isReachable(f, Decision(5, 0)));
    EXPECT_TRUE(PuyoController::isReachable(f, Decision(6, 0)));

    EXPECT_TRUE(PuyoController::isReachable(f, Decision(1, 2)));
    EXPECT_FALSE(PuyoController::isReachable(f, Decision(2, 2)));
    EXPECT_TRUE(PuyoController::isReachable(f, Decision(3, 2)));
    EXPECT_FALSE(PuyoController::isReachable(f, Decision(4, 2)));
    EXPECT_TRUE(PuyoController::isReachable(f, Decision(5, 2)));
    EXPECT_TRUE(PuyoController::isReachable(f, Decision(6, 2)));
}

TEST(PuyoControllerReachabilityTest, wallAboveScreen)
{
    PlainField f(
        " O O  "
        " O O  " // 12
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

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r <= 3; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;

            if (x == 3 && (r == 0 || r == 2))
                EXPECT_TRUE(PuyoController::isReachable(f, d)) << d.toString();
            else
                EXPECT_FALSE(PuyoController::isReachable(f, d)) << d.toString();
        }
    }
}

// A: jiku-puyo
//
// 3 ......
// 2 .@A.@.
// 1 .@..@.
//
TEST(PuyoControllerReachabilityTest, cannotClimbTwoBlocks)
{
    PlainField f(" O  O "
                 " O  O ");

    MovingKumipuyoState mks(KumipuyoPos(3, 2, 0));
    EXPECT_FALSE(PuyoController::isReachableFrom(f, mks, Decision(1, 0)));
    EXPECT_FALSE(PuyoController::isReachableFrom(f, mks, Decision(2, 0)));
    EXPECT_FALSE(PuyoController::isReachableFrom(f, mks, Decision(3, 3)));
    EXPECT_FALSE(PuyoController::isReachableFrom(f, mks, Decision(4, 1)));
    EXPECT_FALSE(PuyoController::isReachableFrom(f, mks, Decision(5, 0)));
    EXPECT_FALSE(PuyoController::isReachableFrom(f, mks, Decision(6, 0)));
}

//
// 14 ......
// 13 @.B.@@
// 12 .@A@..
// 11 ..@...
//
TEST(PuyoControllerReachabilityTest, pivotCannotClimbUpTo14)
{
    PlainField f("O   OO"
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

    EXPECT_FALSE(PuyoController::isReachable(f, Decision(1, 0)));
    EXPECT_FALSE(PuyoController::isReachable(f, Decision(1, 2)));
    EXPECT_FALSE(PuyoController::isReachable(f, Decision(5, 0)));
    EXPECT_FALSE(PuyoController::isReachable(f, Decision(6, 0)));
}

//
// 6  ......
// 5  .....@
// 4  ....@.
// 3  ...@..
// 2  B.@...
// 1  A@....  A: jiku-puyo
TEST(PuyoControllerReachabilityTest, climbStairsRight)
{
    PlainField f("     O"
                 "    OO" // 4
                 "   OOO"
                 "  OOOO"
                 " OOOOO");

    for (int x = 1; x <= 6; x++) {
        EXPECT_TRUE(PuyoController::isReachableFrom(f, MovingKumipuyoState(KumipuyoPos(1, 1, 0)), Decision(x, 0)));
    }
}

//
// 6  @.....
// 5  .@....
// 4  ..@...
// 3  ...@.B
// 2  ....@A
// 1  ....@.  A: jiku-puyo
TEST(PuyoControllerReachabilityTest, climbStairsLeft)
{
    PlainField f("O     "
                 "OO    "
                 "OOO   "
                 "OOOO  "
                 "OOOOO "
                 "OOOOO ");

    for (int x = 1; x <= 6; x++) {
        EXPECT_TRUE(PuyoController::isReachableFrom(f, MovingKumipuyoState(KumipuyoPos(6, 2, 0)), Decision(x, 0)));
    }
}
