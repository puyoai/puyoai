#include "core/puyo_controller.h"

#include <gtest/gtest.h>
#include <vector>

#include "core/decision.h"
#include "core/kumipuyo.h"
#include "core/plain_field.h"

using namespace std;

TEST(PuyoControllerTest, findKeyStrokeOnline)
{
    PlainField f;
    MovingKumipuyoState mks(KumipuyoPos(3, 12, 0));

    EXPECT_EQ("v", PuyoController::findKeyStrokeOnline(f, mks, Decision(3, 0)).toString());
    EXPECT_EQ("<,v", PuyoController::findKeyStrokeOnline(f, mks, Decision(2, 0)).toString());
    EXPECT_EQ(">,>,v", PuyoController::findKeyStrokeOnline(f, mks, Decision(5, 0)).toString());
    EXPECT_EQ(">,>,>,B,,B,v", PuyoController::findKeyStrokeOnline(f, mks, Decision(6, 2)).toString());
    EXPECT_EQ("<,<,A,,A,v", PuyoController::findKeyStrokeOnline(f, mks, Decision(1, 2)).toString());

    EXPECT_EQ("B,,B,>,>,>,>,>,v", PuyoController::findKeyStrokeOnline(f, MovingKumipuyoState(KumipuyoPos(1, 1, 2)), Decision(6, 0)).toString());
}

TEST(PuyoControllerTest, findKeyStrokeByDijkstra)
{
    PlainField f;
    MovingKumipuyoState mks(KumipuyoPos(3, 12, 0));

    EXPECT_EQ("v", PuyoController::findKeyStrokeByDijkstra(f, mks, Decision(3, 0)).toString());
    EXPECT_EQ("<,v", PuyoController::findKeyStrokeByDijkstra(f, mks, Decision(2, 0)).toString());
    EXPECT_EQ("<,<,v", PuyoController::findKeyStrokeByDijkstra(f, mks, Decision(1, 0)).toString());
    EXPECT_EQ(">,v", PuyoController::findKeyStrokeByDijkstra(f, mks, Decision(4, 0)).toString());
    EXPECT_EQ(">,>,v", PuyoController::findKeyStrokeByDijkstra(f, mks, Decision(5, 0)).toString());
    EXPECT_EQ(">,>,>,v", PuyoController::findKeyStrokeByDijkstra(f, mks, Decision(6, 0)).toString());
    EXPECT_EQ("A,v", PuyoController::findKeyStrokeByDijkstra(f, mks, Decision(3, 1)).toString());
    EXPECT_EQ("A,,A,v", PuyoController::findKeyStrokeByDijkstra(f, mks, Decision(3, 2)).toString());
    EXPECT_EQ(">B,>,>B,v", PuyoController::findKeyStrokeByDijkstra(f, mks, Decision(6, 2)).toString());
}

TEST(PuyoControllerTest, nonmovable)
{
    PlainField f(
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
    PlainField f("     O"
                 "    OO" // 4
                 "   OOO"
                 "  OOOO"
                 " OOOOO");

    // TODO(mayah): Actually, dijkstra algorithm can find shorter key stroke.
    const string expected[] = {
        "v",
        "B,,B,,A,,A,v",
        "B,,B,,A,,A,,B,,B,,A,>,,A,v",
        "B,,B,,A,,A,,B,,B,,A,>,,A,,B,,B,,A,>,,A,v",
        "B,,B,,A,,A,,B,,B,,A,>,,A,,B,,B,,A,>,,A,,B,,B,,A,>,,A,v",
        "B,,B,,A,,A,,B,,B,,A,>,,A,,B,,B,,A,>,,A,,B,,B,,A,>,,A,,B,,B,,A,>,,A,v",
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
    PlainField f("O     "
                 "OO    "
                 "OOO   "
                 "OOOO  "
                 "OOOOO "
                 "OOOOO ");

    const string expected[] = {
        "A,,A,,B,,B,,A,,A,,B,<,,B,,A,,A,,B,<,,B,,A,,A,,B,<,,B,,A,,A,,B,<,,B,v",
        "A,,A,,B,,B,,A,,A,,B,<,,B,,A,,A,,B,<,,B,,A,,A,,B,<,,B,v",
        "A,,A,,B,,B,,A,,A,,B,<,,B,,A,,A,,B,<,,B,v",
        "A,,A,,B,,B,,A,,A,,B,<,,B,v",
        "A,,A,,B,,B,v",
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
    PlainField f("    O ");

    MovingKumipuyoState mks(KumipuyoPos(3, 1, 0));
    KeySetSeq kss = PuyoController::findKeyStroke(f, mks, Decision(4, 1));
    EXPECT_EQ(">,B,,B,,B,v", kss.toString());
}
