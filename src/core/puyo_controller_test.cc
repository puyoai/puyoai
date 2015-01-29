#include "core/puyo_controller.h"

#include <gtest/gtest.h>
#include <vector>

#include "core/core_field.h"
#include "core/decision.h"
#include "core/kumipuyo.h"
#include "core/plain_field.h"

using namespace std;

TEST(PuyoControllerTest, findKeyStrokeOnEmptyField)
{
    CoreField f;
    MovingKumipuyoState mks(KumipuyoPos(3, 12, 0));

    EXPECT_EQ("<,,<,v", PuyoController::findKeyStroke(f, mks, Decision(1, 0)).toString());
    EXPECT_EQ("<A,,<,v", PuyoController::findKeyStroke(f, mks, Decision(1, 1)).toString());
    EXPECT_EQ("<A,,<,vA,v", PuyoController::findKeyStroke(f, mks, Decision(1, 2)).toString());

    EXPECT_EQ("<,v", PuyoController::findKeyStroke(f, mks, Decision(2, 0)).toString());
    EXPECT_EQ("<A,v", PuyoController::findKeyStroke(f, mks, Decision(2, 1)).toString());
    EXPECT_EQ("<A,v,vA,v", PuyoController::findKeyStroke(f, mks, Decision(2, 2)).toString());
    EXPECT_EQ("<B,v", PuyoController::findKeyStroke(f, mks, Decision(2, 3)).toString());

    EXPECT_EQ("v", PuyoController::findKeyStroke(f, mks, Decision(3, 0)).toString());
    EXPECT_EQ("vA,v", PuyoController::findKeyStroke(f, mks, Decision(3, 1)).toString());
    EXPECT_EQ("vA,v,vA,v", PuyoController::findKeyStroke(f, mks, Decision(3, 2)).toString());
    EXPECT_EQ("vB,v", PuyoController::findKeyStroke(f, mks, Decision(3, 3)).toString());

    EXPECT_EQ(">,v", PuyoController::findKeyStroke(f, mks, Decision(4, 0)).toString());
    EXPECT_EQ(">A,v", PuyoController::findKeyStroke(f, mks, Decision(4, 1)).toString());
    EXPECT_EQ(">A,v,vA,v", PuyoController::findKeyStroke(f, mks, Decision(4, 2)).toString());
    EXPECT_EQ(">B,v", PuyoController::findKeyStroke(f, mks, Decision(4, 3)).toString());

    EXPECT_EQ(">,,>,v", PuyoController::findKeyStroke(f, mks, Decision(5, 0)).toString());
    EXPECT_EQ(">A,,>,v", PuyoController::findKeyStroke(f, mks, Decision(5, 1)).toString());
    EXPECT_EQ(">A,,>,vA,v", PuyoController::findKeyStroke(f, mks, Decision(5, 2)).toString());
    EXPECT_EQ(">B,,>,v", PuyoController::findKeyStroke(f, mks, Decision(5, 3)).toString());

    EXPECT_EQ(">,,>,,>,v", PuyoController::findKeyStroke(f, mks, Decision(6, 0)).toString());
    EXPECT_EQ(">B,,>,,>B,,v", PuyoController::findKeyStroke(f, mks, Decision(6, 2)).toString());
    EXPECT_EQ(">B,,>,,>,v", PuyoController::findKeyStroke(f, mks, Decision(6, 3)).toString());

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r <= 3; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            EXPECT_TRUE(PuyoController::isReachable(f, d));
            MovingKumipuyoState mks(KumipuyoPos(3, 12, 0));
            KeySetSeq kss = PuyoController::findKeyStroke(f, mks, d);
            for (const auto& ks : kss)
                PuyoController::moveKumipuyo(f, ks, &mks);
            EXPECT_EQ(x, mks.pos.x);
            EXPECT_EQ(r, mks.pos.r);
        }
    }
}

TEST(PuyoControllerTest, findKeyStrokeOnFilledField)
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

    EXPECT_EQ("<,,<,v", PuyoController::findKeyStroke(f, mks, Decision(1, 0)).toString());
    EXPECT_EQ("<A,,<,v", PuyoController::findKeyStroke(f, mks, Decision(1, 1)).toString());
    EXPECT_EQ("<A,,<,A,v", PuyoController::findKeyStroke(f, mks, Decision(1, 2)).toString());

    EXPECT_EQ("<,v", PuyoController::findKeyStroke(f, mks, Decision(2, 0)).toString());
    EXPECT_EQ("<A,v", PuyoController::findKeyStroke(f, mks, Decision(2, 1)).toString());
    EXPECT_EQ("<A,,A,v", PuyoController::findKeyStroke(f, mks, Decision(2, 2)).toString());
    EXPECT_EQ("<B,v", PuyoController::findKeyStroke(f, mks, Decision(2, 3)).toString());

    EXPECT_EQ("v", PuyoController::findKeyStroke(f, mks, Decision(3, 0)).toString());
    EXPECT_EQ("A,v", PuyoController::findKeyStroke(f, mks, Decision(3, 1)).toString());
    EXPECT_EQ("A,,A,v", PuyoController::findKeyStroke(f, mks, Decision(3, 2)).toString());
    EXPECT_EQ("B,v", PuyoController::findKeyStroke(f, mks, Decision(3, 3)).toString());

    EXPECT_EQ(">,v", PuyoController::findKeyStroke(f, mks, Decision(4, 0)).toString());
    EXPECT_EQ(">A,v", PuyoController::findKeyStroke(f, mks, Decision(4, 1)).toString());
    EXPECT_EQ(">A,,A,v", PuyoController::findKeyStroke(f, mks, Decision(4, 2)).toString());
    EXPECT_EQ(">B,v", PuyoController::findKeyStroke(f, mks, Decision(4, 3)).toString());

    EXPECT_EQ(">,,>,v", PuyoController::findKeyStroke(f, mks, Decision(5, 0)).toString());
    EXPECT_EQ(">A,,>,v", PuyoController::findKeyStroke(f, mks, Decision(5, 1)).toString());
    EXPECT_EQ(">A,,>,A,,v", PuyoController::findKeyStroke(f, mks, Decision(5, 2)).toString());
    EXPECT_EQ(">B,,>,v", PuyoController::findKeyStroke(f, mks, Decision(5, 3)).toString());

    EXPECT_EQ(">,,>,,>,v", PuyoController::findKeyStroke(f, mks, Decision(6, 0)).toString());
    EXPECT_EQ(">B,,>,,>B,,v", PuyoController::findKeyStroke(f, mks, Decision(6, 2)).toString());
    EXPECT_EQ(">B,,>,,>,v", PuyoController::findKeyStroke(f, mks, Decision(6, 3)).toString());

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r <= 3; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            EXPECT_TRUE(PuyoController::isReachable(f, d));
            MovingKumipuyoState mks(KumipuyoPos(3, 12, 0));
            KeySetSeq kss = PuyoController::findKeyStroke(f, mks, d);
            for (const auto& ks : kss)
                PuyoController::moveKumipuyo(f, ks, &mks);
            EXPECT_EQ(x, mks.pos.x);
            EXPECT_EQ(r, mks.pos.r);
        }
    }
}

TEST(PuyoControllerTest, findKeyStrokeHigherField1)
{
    CoreField f(
        "......" // 12
        "......"
        ".....O"
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

    EXPECT_EQ("<,,<,v", PuyoController::findKeyStroke(f, mks, Decision(1, 0)).toString());
    EXPECT_EQ("<A,,<,v", PuyoController::findKeyStroke(f, mks, Decision(1, 1)).toString());
    EXPECT_EQ("<A,,<,vA,v", PuyoController::findKeyStroke(f, mks, Decision(1, 2)).toString());

    EXPECT_EQ("<,v", PuyoController::findKeyStroke(f, mks, Decision(2, 0)).toString());
    EXPECT_EQ("<A,v", PuyoController::findKeyStroke(f, mks, Decision(2, 1)).toString());
    EXPECT_EQ("<A,v,vA,v", PuyoController::findKeyStroke(f, mks, Decision(2, 2)).toString());
    EXPECT_EQ("<B,v", PuyoController::findKeyStroke(f, mks, Decision(2, 3)).toString());

    EXPECT_EQ("v", PuyoController::findKeyStroke(f, mks, Decision(3, 0)).toString());
    EXPECT_EQ("vA,v", PuyoController::findKeyStroke(f, mks, Decision(3, 1)).toString());
    EXPECT_EQ("A,,A,v", PuyoController::findKeyStroke(f, mks, Decision(3, 2)).toString());
    EXPECT_EQ("vB,v", PuyoController::findKeyStroke(f, mks, Decision(3, 3)).toString());

    EXPECT_EQ(">,v", PuyoController::findKeyStroke(f, mks, Decision(4, 0)).toString());
    EXPECT_EQ(">A,v", PuyoController::findKeyStroke(f, mks, Decision(4, 1)).toString());
    EXPECT_EQ(">A,v,vA,v", PuyoController::findKeyStroke(f, mks, Decision(4, 2)).toString());
    EXPECT_EQ(">B,v", PuyoController::findKeyStroke(f, mks, Decision(4, 3)).toString());

    EXPECT_EQ(">,,>,v", PuyoController::findKeyStroke(f, mks, Decision(5, 0)).toString());
    EXPECT_EQ(">A,,>,v", PuyoController::findKeyStroke(f, mks, Decision(5, 1)).toString());
    EXPECT_EQ(">A,,>,A,,v", PuyoController::findKeyStroke(f, mks, Decision(5, 2)).toString());
    EXPECT_EQ(">B,,>,v", PuyoController::findKeyStroke(f, mks, Decision(5, 3)).toString());

    EXPECT_EQ(">,,>,,>,v", PuyoController::findKeyStroke(f, mks, Decision(6, 0)).toString());
    EXPECT_EQ(">B,,>,,>B,,v", PuyoController::findKeyStroke(f, mks, Decision(6, 2)).toString());
    EXPECT_EQ(">B,,>,,>,v", PuyoController::findKeyStroke(f, mks, Decision(6, 3)).toString());

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r < 4; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            EXPECT_TRUE(PuyoController::isReachable(f, d));
            MovingKumipuyoState mks(KumipuyoPos::initialPos());
            KeySetSeq kss = PuyoController::findKeyStroke(f, mks, d);
            EXPECT_FALSE(kss.empty());
            for (const auto& ks : kss)
                PuyoController::moveKumipuyo(f, ks, &mks);
            EXPECT_EQ(x, mks.pos.x);
            EXPECT_EQ(r, mks.pos.r);
        }
    }
}

TEST(PuyoControllerTest, findKeyStrokeHigherField2)
{
    CoreField f(
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
    MovingKumipuyoState mks(KumipuyoPos(3, 12, 0));

    set<Decision> unreachable;
    unreachable.insert(Decision(1, 2));
    unreachable.insert(Decision(6, 2));

    EXPECT_EQ("<A,<,<A,<,<B,<,<,<,<B,<,v", PuyoController::findKeyStroke(f, mks, Decision(1, 0)).toString());
    EXPECT_EQ("<A,<,<A,<,<B,<,<,<,v", PuyoController::findKeyStroke(f, mks, Decision(1, 1)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(1, 2)).toString());

    EXPECT_EQ("<,v", PuyoController::findKeyStroke(f, mks, Decision(2, 0)).toString());
    EXPECT_EQ("<A,v", PuyoController::findKeyStroke(f, mks, Decision(2, 1)).toString());
    EXPECT_EQ("<A,,A,v", PuyoController::findKeyStroke(f, mks, Decision(2, 2)).toString());
    EXPECT_EQ("<A,<,<A,<,<A,<,v", PuyoController::findKeyStroke(f, mks, Decision(2, 3)).toString());

    EXPECT_EQ("v", PuyoController::findKeyStroke(f, mks, Decision(3, 0)).toString());
    EXPECT_EQ("A,v", PuyoController::findKeyStroke(f, mks, Decision(3, 1)).toString());
    EXPECT_EQ("A,,A,v", PuyoController::findKeyStroke(f, mks, Decision(3, 2)).toString());
    EXPECT_EQ("B,v", PuyoController::findKeyStroke(f, mks, Decision(3, 3)).toString());

    EXPECT_EQ(">,v", PuyoController::findKeyStroke(f, mks, Decision(4, 0)).toString());
    EXPECT_EQ(">A,v", PuyoController::findKeyStroke(f, mks, Decision(4, 1)).toString());
    EXPECT_EQ(">A,,A,v", PuyoController::findKeyStroke(f, mks, Decision(4, 2)).toString());
    EXPECT_EQ(">B,v", PuyoController::findKeyStroke(f, mks, Decision(4, 3)).toString());

    EXPECT_EQ(">,,>,v", PuyoController::findKeyStroke(f, mks, Decision(5, 0)).toString());
    EXPECT_EQ(">B,>,>B,>,>,>,>B,>,v", PuyoController::findKeyStroke(f, mks, Decision(5, 1)).toString());
    EXPECT_EQ(">B,,>,B,,v", PuyoController::findKeyStroke(f, mks, Decision(5, 2)).toString());
    EXPECT_EQ(">B,,>,v", PuyoController::findKeyStroke(f, mks, Decision(5, 3)).toString());

    EXPECT_EQ(">B,>,>B,>,>A,>,>A,>,v", PuyoController::findKeyStroke(f, mks, Decision(6, 0)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(6, 2)).toString());
    EXPECT_EQ(">B,>,>B,>,>A,>,>,>,v", PuyoController::findKeyStroke(f, mks, Decision(6, 3)).toString());

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r < 4; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            if (unreachable.count(d)) {
                EXPECT_FALSE(PuyoController::isReachable(f, d));
                EXPECT_TRUE(PuyoController::findKeyStroke(f, mks, d).empty());
                continue;
            }
            EXPECT_TRUE(PuyoController::isReachable(f, d));
            MovingKumipuyoState mks(KumipuyoPos::initialPos());
            KeySetSeq kss = PuyoController::findKeyStroke(f, mks, d);
            EXPECT_FALSE(kss.empty());
            for (const auto& ks : kss)
                PuyoController::moveKumipuyo(f, ks, &mks);
            EXPECT_EQ(x, mks.pos.x);
            EXPECT_EQ(r, mks.pos.r);
        }
    }
}

TEST(PuyoControllerTest, findKeyStrokeHigherField3)
{
    CoreField f(
        " O  O " // 12
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

    set<Decision> unreachable;
    unreachable.insert(Decision(2, 2));
    unreachable.insert(Decision(5, 2));

    EXPECT_EQ("<A,<,<A,<,<B,<,<,<,<B,<,v", PuyoController::findKeyStroke(f, mks, Decision(1, 0)).toString());
    EXPECT_EQ("<A,<,<A,<,<B,<,<,<,v", PuyoController::findKeyStroke(f, mks, Decision(1, 1)).toString());
    EXPECT_EQ("<A,<,<A,<,<B,<,<,<,<A,<,v", PuyoController::findKeyStroke(f, mks, Decision(1, 2)).toString());

    EXPECT_EQ("A,,A,,B,<,B,v", PuyoController::findKeyStroke(f, mks, Decision(2, 0)).toString());
    EXPECT_EQ("A,,A,,B,<,v", PuyoController::findKeyStroke(f, mks, Decision(2, 1)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(2, 2)).toString());
    EXPECT_EQ("A,,A,,A,<,v", PuyoController::findKeyStroke(f, mks, Decision(2, 3)).toString());

    EXPECT_EQ("v", PuyoController::findKeyStroke(f, mks, Decision(3, 0)).toString());
    EXPECT_EQ("A,v", PuyoController::findKeyStroke(f, mks, Decision(3, 1)).toString());
    EXPECT_EQ("A,,A,v", PuyoController::findKeyStroke(f, mks, Decision(3, 2)).toString());
    EXPECT_EQ("A,,A,,A,v", PuyoController::findKeyStroke(f, mks, Decision(3, 3)).toString());

    EXPECT_EQ(">,v", PuyoController::findKeyStroke(f, mks, Decision(4, 0)).toString());
    EXPECT_EQ(">,B,,B,,B,v", PuyoController::findKeyStroke(f, mks, Decision(4, 1)).toString());
    EXPECT_EQ(">,B,,B,v", PuyoController::findKeyStroke(f, mks, Decision(4, 2)).toString());
    EXPECT_EQ(">,B,v", PuyoController::findKeyStroke(f, mks, Decision(4, 3)).toString());

    EXPECT_EQ(">B,>,>B,>,>B,>,,B,v", PuyoController::findKeyStroke(f, mks, Decision(5, 0)).toString());
    EXPECT_EQ(">B,>,>B,>,>B,>,>,>,v", PuyoController::findKeyStroke(f, mks, Decision(5, 1)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(5, 2)).toString());
    EXPECT_EQ(">B,,>B,,A,>,v", PuyoController::findKeyStroke(f, mks, Decision(5, 3)).toString());

    EXPECT_EQ(">B,>,>B,>,>A,>,>,>,>A,>,v", PuyoController::findKeyStroke(f, mks, Decision(6, 0)).toString());
    EXPECT_EQ(">B,>,>B,>,>A,>,>,>,>B,>,v", PuyoController::findKeyStroke(f, mks, Decision(6, 2)).toString());
    EXPECT_EQ(">B,>,>B,>,>A,>,>,>,v", PuyoController::findKeyStroke(f, mks, Decision(6, 3)).toString());

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r < 4; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            if (unreachable.count(d)) {
                EXPECT_FALSE(PuyoController::isReachable(f, d));
                EXPECT_TRUE(PuyoController::findKeyStroke(f, mks, d).empty());
                continue;
            }
            EXPECT_TRUE(PuyoController::isReachable(f, d));
            MovingKumipuyoState mks(KumipuyoPos::initialPos());
            KeySetSeq kss = PuyoController::findKeyStroke(f, mks, d);
            EXPECT_FALSE(kss.empty());
            for (const auto& ks : kss)
                PuyoController::moveKumipuyo(f, ks, &mks);
            EXPECT_EQ(x, mks.pos.x);
            EXPECT_EQ(r, mks.pos.r);
        }
    }
}

TEST(PuyoControllerTest, findKeyStrokeHigherField4)
{
    CoreField f(
        "   O  " // 12
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

    set<Decision> unreachable;
    unreachable.insert(Decision(4, 2));

    EXPECT_EQ("<,,<,v", PuyoController::findKeyStroke(f, mks, Decision(1, 0)).toString());
    EXPECT_EQ("<A,,<,v", PuyoController::findKeyStroke(f, mks, Decision(1, 1)).toString());
    EXPECT_EQ("<A,,<,A,v", PuyoController::findKeyStroke(f, mks, Decision(1, 2)).toString());

    EXPECT_EQ("<,v", PuyoController::findKeyStroke(f, mks, Decision(2, 0)).toString());
    EXPECT_EQ("A,v", PuyoController::findKeyStroke(f, mks, Decision(2, 1)).toString());
    EXPECT_EQ("A,,A,v", PuyoController::findKeyStroke(f, mks, Decision(2, 2)).toString());
    EXPECT_EQ("<B,v", PuyoController::findKeyStroke(f, mks, Decision(2, 3)).toString());

    EXPECT_EQ("v", PuyoController::findKeyStroke(f, mks, Decision(3, 0)).toString());
    EXPECT_EQ("B,,B,,B,v", PuyoController::findKeyStroke(f, mks, Decision(3, 1)).toString());
    EXPECT_EQ("B,,B,v", PuyoController::findKeyStroke(f, mks, Decision(3, 2)).toString());
    EXPECT_EQ("B,v", PuyoController::findKeyStroke(f, mks, Decision(3, 3)).toString());

    EXPECT_EQ("B,,B,,B,,>,B,v", PuyoController::findKeyStroke(f, mks, Decision(4, 0)).toString());
    EXPECT_EQ("B,,B,,B,>,v", PuyoController::findKeyStroke(f, mks, Decision(4, 1)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(4, 2)).toString());
    EXPECT_EQ("B,,B,,A,,>,v", PuyoController::findKeyStroke(f, mks, Decision(4, 3)).toString());

    EXPECT_EQ("B,,B,,A,>,A,>,,v", PuyoController::findKeyStroke(f, mks, Decision(5, 0)).toString());
    EXPECT_EQ("B,,B,,B,>,,>,v", PuyoController::findKeyStroke(f, mks, Decision(5, 1)).toString());
    EXPECT_EQ("B,,B,,B,>,,>,A,v", PuyoController::findKeyStroke(f, mks, Decision(5, 2)).toString());
    EXPECT_EQ("B,,B,,A,>,,>,,v", PuyoController::findKeyStroke(f, mks, Decision(5, 3)).toString());

    EXPECT_EQ(">B,>,>B,>,>A,>,>,>,>A,>,v", PuyoController::findKeyStroke(f, mks, Decision(6, 0)).toString());
    EXPECT_EQ(">B,>,>B,>,>A,>,>,>,>B,>,v", PuyoController::findKeyStroke(f, mks, Decision(6, 2)).toString());
    EXPECT_EQ(">B,>,>B,>,>A,>,>,>,>,>,v", PuyoController::findKeyStroke(f, mks, Decision(6, 3)).toString());

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r < 4; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            if (unreachable.count(d)) {
                EXPECT_FALSE(PuyoController::isReachable(f, d));
                EXPECT_TRUE(PuyoController::findKeyStroke(f, mks, d).empty());
                continue;
            }
            EXPECT_TRUE(PuyoController::isReachable(f, d));
            MovingKumipuyoState mks(KumipuyoPos::initialPos());
            KeySetSeq kss = PuyoController::findKeyStroke(f, mks, d);
            EXPECT_FALSE(kss.empty());
            for (const auto& ks : kss)
                PuyoController::moveKumipuyo(f, ks, &mks);
            EXPECT_EQ(x, mks.pos.x) << d.toString();
            EXPECT_EQ(r, mks.pos.r);
        }
    }
}

TEST(PuyoControllerTest, findKeyStrokeHigherField5)
{
    CoreField f(
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
    MovingKumipuyoState mks(KumipuyoPos(3, 12, 0));

    set<Decision> unreachable;
    unreachable.insert(Decision(2, 2));
    unreachable.insert(Decision(4, 2));

    EXPECT_EQ("<A,<,<A,<,<B,<,<,<,<B,<,v", PuyoController::findKeyStroke(f, mks, Decision(1, 0)).toString());
    EXPECT_EQ("<A,<,<A,<,<B,<,<,<,v", PuyoController::findKeyStroke(f, mks, Decision(1, 1)).toString());
    EXPECT_EQ("<A,<,<A,<,<B,<,<,<,<A,<,v", PuyoController::findKeyStroke(f, mks, Decision(1, 2)).toString());

    EXPECT_EQ("A,,A,,B,<,B,v", PuyoController::findKeyStroke(f, mks, Decision(2, 0)).toString());
    EXPECT_EQ("A,,A,,B,<,v", PuyoController::findKeyStroke(f, mks, Decision(2, 1)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(2, 2)).toString());
    EXPECT_EQ("A,,A,,A,<,v", PuyoController::findKeyStroke(f, mks, Decision(2, 3)).toString());

    EXPECT_EQ("v", PuyoController::findKeyStroke(f, mks, Decision(3, 0)).toString());
    EXPECT_EQ("B,,B,,B,v", PuyoController::findKeyStroke(f, mks, Decision(3, 1)).toString());
    EXPECT_EQ("A,,A,v", PuyoController::findKeyStroke(f, mks, Decision(3, 2)).toString());
    EXPECT_EQ("A,,A,,A,v", PuyoController::findKeyStroke(f, mks, Decision(3, 3)).toString());

    EXPECT_EQ("A,,A,,A,>,A,v", PuyoController::findKeyStroke(f, mks, Decision(4, 0)).toString());
    EXPECT_EQ("B,,B,,B,>,v", PuyoController::findKeyStroke(f, mks, Decision(4, 1)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(4, 2)).toString());
    EXPECT_EQ("A,,A,,A,>,v", PuyoController::findKeyStroke(f, mks, Decision(4, 3)).toString());

    EXPECT_EQ("B,,B,,A,>,A,>,,v", PuyoController::findKeyStroke(f, mks, Decision(5, 0)).toString());
    EXPECT_EQ("B,,B,,B,>,,>,v", PuyoController::findKeyStroke(f, mks, Decision(5, 1)).toString());
    EXPECT_EQ("B,,B,,B,>,,>,A,v", PuyoController::findKeyStroke(f, mks, Decision(5, 2)).toString());
    EXPECT_EQ("B,,B,,A,>,,>,,v", PuyoController::findKeyStroke(f, mks, Decision(5, 3)).toString());

    EXPECT_EQ("A,,A,,A,>,,>,,>,A,v", PuyoController::findKeyStroke(f, mks, Decision(6, 0)).toString());
    EXPECT_EQ("A,,A,,A,>,,>,,>,B,v", PuyoController::findKeyStroke(f, mks, Decision(6, 2)).toString());
    EXPECT_EQ("A,,A,,A,>,,>,,>,v", PuyoController::findKeyStroke(f, mks, Decision(6, 3)).toString());

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r < 4; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            if (unreachable.count(d)) {
                EXPECT_FALSE(PuyoController::isReachable(f, d));
                EXPECT_TRUE(PuyoController::findKeyStroke(f, mks, d).empty());
                continue;
            }
            EXPECT_TRUE(PuyoController::isReachable(f, d));
            MovingKumipuyoState mks(KumipuyoPos::initialPos());
            KeySetSeq kss = PuyoController::findKeyStroke(f, mks, d);
            EXPECT_FALSE(kss.empty());
            for (const auto& ks : kss)
                PuyoController::moveKumipuyo(f, ks, &mks);
            EXPECT_EQ(x, mks.pos.x);
            EXPECT_EQ(r, mks.pos.r);
        }
    }
}

TEST(PuyoControllerTest, findKeyStrokeHigherField6)
{
    CoreField f(
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

    set<Decision> unreachable;
    unreachable.insert(Decision(2, 2));
    unreachable.insert(Decision(4, 2));

    EXPECT_EQ("<A,<,<A,<,<B,<,<,<,<B,<,v", PuyoController::findKeyStroke(f, mks, Decision(1, 0)).toString());
    EXPECT_EQ("<A,<,<A,<,<B,<,<,<,v", PuyoController::findKeyStroke(f, mks, Decision(1, 1)).toString());
    EXPECT_EQ("<A,<,<A,<,<B,<,<,<,<A,<,v", PuyoController::findKeyStroke(f, mks, Decision(1, 2)).toString());

    EXPECT_EQ("A,,A,,B,<,B,v", PuyoController::findKeyStroke(f, mks, Decision(2, 0)).toString());
    EXPECT_EQ("A,,A,,B,<,v", PuyoController::findKeyStroke(f, mks, Decision(2, 1)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(2, 2)).toString());
    EXPECT_EQ("A,,A,,A,<,v", PuyoController::findKeyStroke(f, mks, Decision(2, 3)).toString());

    EXPECT_EQ("v", PuyoController::findKeyStroke(f, mks, Decision(3, 0)).toString());
    EXPECT_EQ("B,,B,,B,v", PuyoController::findKeyStroke(f, mks, Decision(3, 1)).toString());
    EXPECT_EQ("A,,A,v", PuyoController::findKeyStroke(f, mks, Decision(3, 2)).toString());
    EXPECT_EQ("A,,A,,A,v", PuyoController::findKeyStroke(f, mks, Decision(3, 3)).toString());

    EXPECT_EQ("A,,A,,A,>,A,v", PuyoController::findKeyStroke(f, mks, Decision(4, 0)).toString());
    EXPECT_EQ("B,,B,,A,>,A,,A,v", PuyoController::findKeyStroke(f, mks, Decision(4, 1)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(4, 2)).toString());
    EXPECT_EQ("A,,A,,A,>,v", PuyoController::findKeyStroke(f, mks, Decision(4, 3)).toString());

    EXPECT_EQ("B,,B,,A,>,A,>,,v", PuyoController::findKeyStroke(f, mks, Decision(5, 0)).toString());
    EXPECT_EQ("B,,B,,A,>,A,>,A,v", PuyoController::findKeyStroke(f, mks, Decision(5, 1)).toString());
    EXPECT_EQ("B,,B,,A,>,A,>,B,,B,v", PuyoController::findKeyStroke(f, mks, Decision(5, 2)).toString());
    EXPECT_EQ("B,,B,,A,>,A,>,B,v", PuyoController::findKeyStroke(f, mks, Decision(5, 3)).toString());

    EXPECT_EQ("A,,A,,A,>,,>,,>,A,v", PuyoController::findKeyStroke(f, mks, Decision(6, 0)).toString());
    EXPECT_EQ("A,,A,,A,>,,>,,>,B,v", PuyoController::findKeyStroke(f, mks, Decision(6, 2)).toString());
    EXPECT_EQ("A,,A,,A,>,,>,,>,v", PuyoController::findKeyStroke(f, mks, Decision(6, 3)).toString());

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r < 4; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            if (unreachable.count(d)) {
                EXPECT_FALSE(PuyoController::isReachable(f, d));
                EXPECT_TRUE(PuyoController::findKeyStroke(f, mks, d).empty());
                continue;
            }
            EXPECT_TRUE(PuyoController::isReachable(f, d)) << d.toString();
            MovingKumipuyoState mks(KumipuyoPos::initialPos());
            KeySetSeq kss = PuyoController::findKeyStroke(f, mks, d);
            EXPECT_FALSE(kss.empty()) << d.toString();
            for (const auto& ks : kss)
                PuyoController::moveKumipuyo(f, ks, &mks);
            EXPECT_EQ(x, mks.pos.x) << d.toString();
            EXPECT_EQ(r, mks.pos.r) << d.toString();
        }
    }
}

TEST(PuyoControllerTest, findKeyStrokeHigherField7)
{
    CoreField f(
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
    MovingKumipuyoState mks(KumipuyoPos(3, 12, 0));

    set<Decision> reachable;
    reachable.insert(Decision(3, 0));
    reachable.insert(Decision(3, 2));

    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(1, 0)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(1, 1)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(1, 2)).toString());

    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(2, 0)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(2, 1)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(2, 2)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(2, 3)).toString());

    EXPECT_EQ("v", PuyoController::findKeyStroke(f, mks, Decision(3, 0)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(3, 1)).toString());
    EXPECT_EQ("A,,A,v", PuyoController::findKeyStroke(f, mks, Decision(3, 2)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(3, 3)).toString());

    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(4, 0)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(4, 1)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(4, 2)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(4, 3)).toString());

    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(5, 0)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(5, 1)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(5, 2)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(5, 3)).toString());

    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(6, 0)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(6, 2)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(6, 3)).toString());

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r < 4; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            if (!reachable.count(d)) {
                EXPECT_FALSE(PuyoController::isReachable(f, d))
                    << d.toString() << " " << PuyoController::findKeyStroke(f, mks, d).toString();
                EXPECT_TRUE(PuyoController::findKeyStroke(f, mks, d).empty()) << d.toString();
                continue;
            }
            EXPECT_TRUE(PuyoController::isReachable(f, d));
            MovingKumipuyoState mks(KumipuyoPos::initialPos());
            KeySetSeq kss = PuyoController::findKeyStroke(f, mks, d);
            EXPECT_FALSE(kss.empty());
            for (const auto& ks : kss)
                PuyoController::moveKumipuyo(f, ks, &mks);
            EXPECT_EQ(x, mks.pos.x);
            EXPECT_EQ(r, mks.pos.r);
        }
    }
}

TEST(PuyoControllerTest, findKeyStrokeHigherField8)
{
    CoreField f(
        "O   OO"
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
    MovingKumipuyoState mks(KumipuyoPos(3, 12, 0));

    set<Decision> reachable;
    reachable.insert(Decision(2, 0));
    reachable.insert(Decision(2, 1));
    reachable.insert(Decision(3, 0));
    reachable.insert(Decision(3, 1));
    reachable.insert(Decision(3, 2));
    reachable.insert(Decision(3, 3));
    reachable.insert(Decision(4, 0));
    reachable.insert(Decision(4, 3));

    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(1, 0)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(1, 1)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(1, 2)).toString());

    EXPECT_EQ("A,,A,,B,<,B,v", PuyoController::findKeyStroke(f, mks, Decision(2, 0)).toString());
    EXPECT_EQ("A,,A,,B,<,v", PuyoController::findKeyStroke(f, mks, Decision(2, 1)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(2, 2)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(2, 3)).toString());

    EXPECT_EQ("v", PuyoController::findKeyStroke(f, mks, Decision(3, 0)).toString());
    EXPECT_EQ("B,,B,,B,v", PuyoController::findKeyStroke(f, mks, Decision(3, 1)).toString());
    EXPECT_EQ("A,,A,v", PuyoController::findKeyStroke(f, mks, Decision(3, 2)).toString());
    EXPECT_EQ("A,,A,,A,v", PuyoController::findKeyStroke(f, mks, Decision(3, 3)).toString());

    EXPECT_EQ("A,,A,,A,>,A,v", PuyoController::findKeyStroke(f, mks, Decision(4, 0)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(4, 1)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(4, 2)).toString());
    EXPECT_EQ("A,,A,,A,>,v", PuyoController::findKeyStroke(f, mks, Decision(4, 3)).toString());

    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(5, 0)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(5, 1)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(5, 2)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(5, 3)).toString());

    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(6, 0)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(6, 2)).toString());
    EXPECT_EQ("", PuyoController::findKeyStroke(f, mks, Decision(6, 3)).toString());

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r < 4; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            if (!reachable.count(d)) {
                EXPECT_FALSE(PuyoController::isReachable(f, d));
                EXPECT_TRUE(PuyoController::findKeyStroke(f, mks, d).empty());
                continue;
            }
            EXPECT_TRUE(PuyoController::isReachable(f, d));
            MovingKumipuyoState mks(KumipuyoPos::initialPos());
            KeySetSeq kss = PuyoController::findKeyStroke(f, mks, d);
            EXPECT_FALSE(kss.empty());
            for (const auto& ks : kss)
                PuyoController::moveKumipuyo(f, ks, &mks);
            EXPECT_EQ(x, mks.pos.x);
            EXPECT_EQ(r, mks.pos.r);
        }
    }
}
