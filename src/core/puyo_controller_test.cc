#include "core/puyo_controller.h"

#include <gtest/gtest.h>
#include <vector>

#include "core/core_field.h"
#include "core/decision.h"
#include "core/kumipuyo.h"
#include "core/plain_field.h"

using namespace std;

TEST(PuyoControllerTest, findKeyStrokeFastpathOnEmptyField)
{
    CoreField f;
    MovingKumipuyoState mks(KumipuyoPos(3, 12, 0));

    EXPECT_EQ("<,,<,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(1, 0)).toString());
    EXPECT_EQ("<A,,<,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(1, 1)).toString());
    EXPECT_EQ("<A,,<,vA,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(1, 2)).toString());

    EXPECT_EQ("<,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(2, 0)).toString());
    EXPECT_EQ("<A,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(2, 1)).toString());
    EXPECT_EQ("<A,v,vA,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(2, 2)).toString());
    EXPECT_EQ("<B,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(2, 3)).toString());

    EXPECT_EQ("v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(3, 0)).toString());
    EXPECT_EQ("vA,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(3, 1)).toString());
    EXPECT_EQ("vA,v,vA,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(3, 2)).toString());
    EXPECT_EQ("vB,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(3, 3)).toString());

    EXPECT_EQ(">,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(4, 0)).toString());
    EXPECT_EQ(">A,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(4, 1)).toString());
    EXPECT_EQ(">A,v,vA,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(4, 2)).toString());
    EXPECT_EQ(">B,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(4, 3)).toString());

    EXPECT_EQ(">,,>,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(5, 0)).toString());
    EXPECT_EQ(">A,,>,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(5, 1)).toString());
    EXPECT_EQ(">A,,>,vA,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(5, 2)).toString());
    EXPECT_EQ(">B,,>,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(5, 3)).toString());

    EXPECT_EQ(">,,>,,>,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(6, 0)).toString());
    EXPECT_EQ(">B,,>,,>B,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(6, 2)).toString());
    EXPECT_EQ(">B,,>,,>,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(6, 3)).toString());

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r <= 3; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            MovingKumipuyoState mks(KumipuyoPos(3, 12, 0));
            KeySetSeq kss = PuyoController::findKeyStrokeFastpath(f, mks, d);
            for (const auto& ks : kss)
                PuyoController::moveKumipuyo(f, ks, &mks);
            EXPECT_EQ(x, mks.pos.x);
            EXPECT_EQ(r, mks.pos.r);
        }
    }
}

TEST(PuyoControllerTest, findKeyStrokeFastpathOnFilledField)
{
    CoreField f(
        "......" // 12
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

    MovingKumipuyoState mks(KumipuyoPos(3, 12, 0));

    EXPECT_EQ("<,,<,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(1, 0)).toString());
    EXPECT_EQ("<A,,<,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(1, 1)).toString());
    EXPECT_EQ("<A,,<,A,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(1, 2)).toString());

    EXPECT_EQ("<,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(2, 0)).toString());
    EXPECT_EQ("<A,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(2, 1)).toString());
    EXPECT_EQ("<A,,A,,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(2, 2)).toString());
    EXPECT_EQ("<B,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(2, 3)).toString());

    EXPECT_EQ("v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(3, 0)).toString());
    EXPECT_EQ("A,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(3, 1)).toString());
    EXPECT_EQ("A,,A,,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(3, 2)).toString());
    EXPECT_EQ("B,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(3, 3)).toString());

    EXPECT_EQ(">,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(4, 0)).toString());
    EXPECT_EQ(">A,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(4, 1)).toString());
    EXPECT_EQ(">A,,A,,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(4, 2)).toString());
    EXPECT_EQ(">B,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(4, 3)).toString());

    EXPECT_EQ(">,,>,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(5, 0)).toString());
    EXPECT_EQ(">A,,>,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(5, 1)).toString());
    EXPECT_EQ(">A,,>,A,,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(5, 2)).toString());
    EXPECT_EQ(">B,,>,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(5, 3)).toString());

    EXPECT_EQ(">,,>,,>,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(6, 0)).toString());
    EXPECT_EQ(">B,,>,,>B,,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(6, 2)).toString());
    EXPECT_EQ(">B,,>,,>,v", PuyoController::findKeyStrokeFastpath(f, mks, Decision(6, 3)).toString());

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r <= 3; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            MovingKumipuyoState mks(KumipuyoPos(3, 12, 0));
            KeySetSeq kss = PuyoController::findKeyStrokeFastpath(f, mks, d);
            for (const auto& ks : kss)
                PuyoController::moveKumipuyo(f, ks, &mks);
            EXPECT_EQ(x, mks.pos.x);
            EXPECT_EQ(r, mks.pos.r);
        }
    }
}

TEST(PuyoControllerTest, findKeyStrokeOnlineOnEmptyField)
{
    CoreField f;
    MovingKumipuyoState mks(KumipuyoPos(3, 12, 0));

    EXPECT_EQ("v", PuyoController::findKeyStrokeOnline(f, mks, Decision(3, 0)).toString());
    EXPECT_EQ("<,v", PuyoController::findKeyStrokeOnline(f, mks, Decision(2, 0)).toString());
    EXPECT_EQ(">,>,v", PuyoController::findKeyStrokeOnline(f, mks, Decision(5, 0)).toString());
    EXPECT_EQ(">,>,>,B,,B,v", PuyoController::findKeyStrokeOnline(f, mks, Decision(6, 2)).toString());
    EXPECT_EQ("<,<,A,,A,v", PuyoController::findKeyStrokeOnline(f, mks, Decision(1, 2)).toString());

    EXPECT_EQ("B,,B,>,>,>,>,>,v", PuyoController::findKeyStrokeOnline(f, MovingKumipuyoState(KumipuyoPos(1, 1, 2)), Decision(6, 0)).toString());
}

TEST(PuyoControllerTest, findKeyStrokeByDijkstraOnEmptyField)
{
    CoreField f;
    MovingKumipuyoState mks(KumipuyoPos(3, 12, 0));

    EXPECT_EQ("v", PuyoController::findKeyStrokeByDijkstra(f, mks, Decision(3, 0)).toString());
    EXPECT_EQ("<,v", PuyoController::findKeyStrokeByDijkstra(f, mks, Decision(2, 0)).toString());
    EXPECT_EQ("<,,<,v", PuyoController::findKeyStrokeByDijkstra(f, mks, Decision(1, 0)).toString());
    EXPECT_EQ(">,v", PuyoController::findKeyStrokeByDijkstra(f, mks, Decision(4, 0)).toString());
    EXPECT_EQ(">,,>,v", PuyoController::findKeyStrokeByDijkstra(f, mks, Decision(5, 0)).toString());
    EXPECT_EQ(">,,>,,>,v", PuyoController::findKeyStrokeByDijkstra(f, mks, Decision(6, 0)).toString());
    EXPECT_EQ("A,v", PuyoController::findKeyStrokeByDijkstra(f, mks, Decision(3, 1)).toString());
    EXPECT_EQ("B,,B,v", PuyoController::findKeyStrokeByDijkstra(f, mks, Decision(3, 2)).toString());
    EXPECT_EQ(">,B,>,B,>,v", PuyoController::findKeyStrokeByDijkstra(f, mks, Decision(6, 2)).toString());
}

TEST(PuyoControllerTest, nonmovable)
{
    CoreField f(
        " O O  "
        " O O  " // 12
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
    MovingKumipuyoState mks(KumipuyoPos(3, 12, 0));

    EXPECT_EQ("", PuyoController::findKeyStrokeFastpath(f, mks, Decision(6, 2)).toString());
    EXPECT_EQ("", PuyoController::findKeyStrokeOnline(f, mks, Decision(6, 2)).toString());
    EXPECT_EQ("", PuyoController::findKeyStrokeByDijkstra(f, mks, Decision(6, 2)).toString());
}

// A: jiku-puyo
//
// 6  ......
// 5  .....@
// 4  ....@.
// 3  ...@..
// 2  B.@...
// 1  A@....
//
TEST(PuyoControllerTest, climbStairsRight)
{
    CoreField f("     O"
                "    OO" // 4
                "   OOO"
                "  OOOO"
                " OOOOO");

    // TODO(mayah): Actually, dijkstra algorithm can find shorter key stroke.
    const string expected[] = {
        "v",
        "A,,B,,A,,A,v",
        "A,,B,,A,,B,,A,>,A,v",
        "A,,B,,A,,B,,A,>,B,,A,>,A,v",
        "A,,B,,A,,B,,A,>,B,,A,>,B,,A,>,A,v",
        "A,,B,,A,,B,,A,>,B,,A,>,B,,A,>,B,,A,>,A,v"
    };

    for (int x = 1; x <= 6; ++x) {
        MovingKumipuyoState mks(KumipuyoPos(1, 1, 0));
        KeySetSeq kss = PuyoController::findKeyStroke(f, mks, Decision(x, 0));
        EXPECT_EQ(expected[x - 1], kss.toString());
    }
}

//
// 6  @.....
// 5  .@....
// 4  ..@...
// 3  ...@.B
// 2  ....@A
// 1  ....@.  A: jiku-puyo
//
TEST(PuyoControllerTest, climbStairsLeft)
{
    CoreField f("O     "
                "OO    "
                "OOO   "
                "OOOO  "
                "OOOOO "
                "OOOOO ");

    const string expected[] = {
        "A,,B,,B,,A,,B,<,A,,B,<,A,,B,<,A,,B,<,B,v",
        "A,,B,,B,,A,,B,<,A,,B,<,A,,B,<,B,v",
        "A,,B,,B,,A,,B,<,A,,B,<,B,v",
        "A,,B,,B,,A,,B,<,B,v",
        "A,,B,,B,,B,v",
        "v",
    };

    for (int x = 1; x <= 6; ++x) {
        MovingKumipuyoState mks(KumipuyoPos(6, 2, 0));
        KeySetSeq kss = PuyoController::findKeyStroke(f, mks, Decision(x, 0));
        EXPECT_EQ(expected[x - 1], kss.toString()) << x;
    }
}

TEST(PuyoControllerTest, subpuyoIsHigher)
{
    CoreField f("    O ");

    MovingKumipuyoState mks(KumipuyoPos(3, 1, 0));
    KeySetSeq kss = PuyoController::findKeyStroke(f, mks, Decision(4, 1));
    EXPECT_EQ("B,,B,>,B,v", kss.toString());
}

TEST(PuyoControllerTest, checkHigherField1)
{
    CoreField f(
        "      " // 12
        "  OO  "
        " OOOO "
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
        for (int r = 0; r < 4; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            MovingKumipuyoState mks(KumipuyoPos::initialPos());
            KeySetSeq kss = PuyoController::findKeyStrokeFastpath(f, mks, d);
            if (kss.empty())
                continue;
            for (const auto& ks : kss) {
                PuyoController::moveKumipuyo(f, ks, &mks);
            }
            EXPECT_EQ(x, mks.pos.x);
            EXPECT_EQ(r, mks.pos.r);
        }
    }
}

TEST(PuyoControllerTest, checkHigherField2)
{
    CoreField f(
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
        for (int r = 0; r < 4; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            MovingKumipuyoState mks(KumipuyoPos::initialPos());
            KeySetSeq kss = PuyoController::findKeyStrokeFastpath(f, mks, d);
            if (kss.empty())
                continue;
            for (const auto& ks : kss) {
                PuyoController::moveKumipuyo(f, ks, &mks);
            }
            EXPECT_EQ(x, mks.pos.x);
            EXPECT_EQ(r, mks.pos.r);
        }
    }
}

TEST(PuyoControllerTest, rightHigherField)
{
    CoreField f(
        "      " // 12
        "      "
        "     O"
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    // This causes miscontrolling on wii.
    MovingKumipuyoState mks(KumipuyoPos::initialPos());
    KeySetSeq kss = PuyoController::findKeyStroke(f, mks, Decision(6, 2));
    EXPECT_EQ(">B,,>,,>B,,v", kss.toString());
}
