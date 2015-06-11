#include "core/puyo_controller.h"

#include <gtest/gtest.h>

#include <set>
#include <string>

#include "core/core_field.h"
#include "core/decision.h"
#include "core/kumipuyo_pos.h"
#include "core/kumipuyo_moving_state.h"
#include "core/puyo_color.h"

using namespace std;

TEST(PuyoControllerTest, findKeyStrokeOnEmptyField)
{
    CoreField f;

    EXPECT_EQ(KeySetSeq("<,,<,v"), PuyoController::findKeyStroke(f, Decision(1, 0)).seq());
    EXPECT_EQ(KeySetSeq("<A,,<,v"), PuyoController::findKeyStroke(f, Decision(1, 1)).seq());
    EXPECT_EQ(KeySetSeq("<A,,<,vA,v"), PuyoController::findKeyStroke(f, Decision(1, 2)).seq());

    EXPECT_EQ(KeySetSeq("<,v"), PuyoController::findKeyStroke(f, Decision(2, 0)).seq());
    EXPECT_EQ(KeySetSeq("<A,v"), PuyoController::findKeyStroke(f, Decision(2, 1)).seq());
    EXPECT_EQ(KeySetSeq("<A,v,vA,v"), PuyoController::findKeyStroke(f, Decision(2, 2)).seq());
    EXPECT_EQ(KeySetSeq("<B,v"), PuyoController::findKeyStroke(f, Decision(2, 3)).seq());

    EXPECT_EQ(KeySetSeq("v"), PuyoController::findKeyStroke(f, Decision(3, 0)).seq());
    EXPECT_EQ(KeySetSeq("vA,v"), PuyoController::findKeyStroke(f, Decision(3, 1)).seq());
    EXPECT_EQ(KeySetSeq("vA,v,vA,v"), PuyoController::findKeyStroke(f, Decision(3, 2)).seq());
    EXPECT_EQ(KeySetSeq("vB,v"), PuyoController::findKeyStroke(f, Decision(3, 3)).seq());

    EXPECT_EQ(KeySetSeq(">,v"), PuyoController::findKeyStroke(f, Decision(4, 0)).seq());
    EXPECT_EQ(KeySetSeq(">A,v"), PuyoController::findKeyStroke(f, Decision(4, 1)).seq());
    EXPECT_EQ(KeySetSeq(">A,v,vA,v"), PuyoController::findKeyStroke(f, Decision(4, 2)).seq());
    EXPECT_EQ(KeySetSeq(">B,v"), PuyoController::findKeyStroke(f, Decision(4, 3)).seq());

    EXPECT_EQ(KeySetSeq(">,,>,v"), PuyoController::findKeyStroke(f, Decision(5, 0)).seq());
    EXPECT_EQ(KeySetSeq(">A,,>,v"), PuyoController::findKeyStroke(f, Decision(5, 1)).seq());
    EXPECT_EQ(KeySetSeq(">A,,>,vA,v"), PuyoController::findKeyStroke(f, Decision(5, 2)).seq());
    EXPECT_EQ(KeySetSeq(">B,,>,v"), PuyoController::findKeyStroke(f, Decision(5, 3)).seq());

    EXPECT_EQ(KeySetSeq(">,,>,,>,v"), PuyoController::findKeyStroke(f, Decision(6, 0)).seq());
    EXPECT_EQ(KeySetSeq(">B,,>,,>,B,v"), PuyoController::findKeyStroke(f, Decision(6, 2)).seq());
    EXPECT_EQ(KeySetSeq(">B,,>,,>,v"), PuyoController::findKeyStroke(f, Decision(6, 3)).seq());

    PlainField pf = f.toPlainField();
    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r <= 3; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            EXPECT_TRUE(PuyoController::isReachable(f, d));
            KumipuyoMovingState kms(KumipuyoPos(3, 12, 0));
            KeySetSeq kss = PuyoController::findKeyStroke(f, d).seq();
            for (const auto& ks : kss)
                kms.moveKumipuyo(pf, ks);
            EXPECT_EQ(x, kms.pos.x);
            EXPECT_EQ(r, kms.pos.r);
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

    KumipuyoMovingState kms(KumipuyoPos(3, 12, 0));

    EXPECT_EQ(KeySetSeq("<,,<,v"), PuyoController::findKeyStroke(f, Decision(1, 0)).seq());
    EXPECT_EQ(KeySetSeq("<A,,<,v"), PuyoController::findKeyStroke(f, Decision(1, 1)).seq());
    EXPECT_EQ(KeySetSeq("<A,,<,A,v"), PuyoController::findKeyStroke(f, Decision(1, 2)).seq());

    EXPECT_EQ(KeySetSeq("<,v"), PuyoController::findKeyStroke(f, Decision(2, 0)).seq());
    EXPECT_EQ(KeySetSeq("<A,v"), PuyoController::findKeyStroke(f, Decision(2, 1)).seq());
    EXPECT_EQ(KeySetSeq("<A,,A,v"), PuyoController::findKeyStroke(f, Decision(2, 2)).seq());
    EXPECT_EQ(KeySetSeq("<B,v"), PuyoController::findKeyStroke(f, Decision(2, 3)).seq());

    EXPECT_EQ(KeySetSeq("v"), PuyoController::findKeyStroke(f, Decision(3, 0)).seq());
    EXPECT_EQ(KeySetSeq("A,v"), PuyoController::findKeyStroke(f, Decision(3, 1)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,v"), PuyoController::findKeyStroke(f, Decision(3, 2)).seq());
    EXPECT_EQ(KeySetSeq("B,v"), PuyoController::findKeyStroke(f, Decision(3, 3)).seq());

    EXPECT_EQ(KeySetSeq(">,v"), PuyoController::findKeyStroke(f, Decision(4, 0)).seq());
    EXPECT_EQ(KeySetSeq(">A,v"), PuyoController::findKeyStroke(f, Decision(4, 1)).seq());
    EXPECT_EQ(KeySetSeq(">A,,A,v"), PuyoController::findKeyStroke(f, Decision(4, 2)).seq());
    EXPECT_EQ(KeySetSeq(">B,v"), PuyoController::findKeyStroke(f, Decision(4, 3)).seq());

    EXPECT_EQ(KeySetSeq(">,,>,v"), PuyoController::findKeyStroke(f, Decision(5, 0)).seq());
    EXPECT_EQ(KeySetSeq(">A,,>,v"), PuyoController::findKeyStroke(f, Decision(5, 1)).seq());
    EXPECT_EQ(KeySetSeq(">A,,>,A,,v"), PuyoController::findKeyStroke(f, Decision(5, 2)).seq());
    EXPECT_EQ(KeySetSeq(">B,,>,v"), PuyoController::findKeyStroke(f, Decision(5, 3)).seq());

    EXPECT_EQ(KeySetSeq(">,,>,,>,v"), PuyoController::findKeyStroke(f, Decision(6, 0)).seq());
    EXPECT_EQ(KeySetSeq(">B,,>,,>B,,v"), PuyoController::findKeyStroke(f, Decision(6, 2)).seq());
    EXPECT_EQ(KeySetSeq(">B,,>,,>,v"), PuyoController::findKeyStroke(f, Decision(6, 3)).seq());

    PlainField pf = f.toPlainField();
    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r <= 3; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            EXPECT_TRUE(PuyoController::isReachable(f, d));
            KumipuyoMovingState kms(KumipuyoPos(3, 12, 0));
            KeySetSeq kss = PuyoController::findKeyStroke(f, d).seq();
            for (const auto& ks : kss)
                kms.moveKumipuyo(pf, ks);
            EXPECT_EQ(x, kms.pos.x);
            EXPECT_EQ(r, kms.pos.r);
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
    KumipuyoMovingState kms(KumipuyoPos(3, 12, 0));

    EXPECT_EQ(KeySetSeq("<,,<,v"), PuyoController::findKeyStroke(f, Decision(1, 0)).seq());
    EXPECT_EQ(KeySetSeq("<A,,<,v"), PuyoController::findKeyStroke(f, Decision(1, 1)).seq());
    EXPECT_EQ(KeySetSeq("<A,,<,vA,v"), PuyoController::findKeyStroke(f, Decision(1, 2)).seq());

    EXPECT_EQ(KeySetSeq("<,v"), PuyoController::findKeyStroke(f, Decision(2, 0)).seq());
    EXPECT_EQ(KeySetSeq("<A,v"), PuyoController::findKeyStroke(f, Decision(2, 1)).seq());
    EXPECT_EQ(KeySetSeq("<A,v,vA,v"), PuyoController::findKeyStroke(f, Decision(2, 2)).seq());
    EXPECT_EQ(KeySetSeq("<B,v"), PuyoController::findKeyStroke(f, Decision(2, 3)).seq());

    EXPECT_EQ(KeySetSeq("v"), PuyoController::findKeyStroke(f, Decision(3, 0)).seq());
    EXPECT_EQ(KeySetSeq("vA,v"), PuyoController::findKeyStroke(f, Decision(3, 1)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,v"), PuyoController::findKeyStroke(f, Decision(3, 2)).seq());
    EXPECT_EQ(KeySetSeq("vB,v"), PuyoController::findKeyStroke(f, Decision(3, 3)).seq());

    EXPECT_EQ(KeySetSeq(">,v"), PuyoController::findKeyStroke(f, Decision(4, 0)).seq());
    EXPECT_EQ(KeySetSeq(">A,v"), PuyoController::findKeyStroke(f, Decision(4, 1)).seq());
    EXPECT_EQ(KeySetSeq(">A,v,vA,v"), PuyoController::findKeyStroke(f, Decision(4, 2)).seq());
    EXPECT_EQ(KeySetSeq(">B,v"), PuyoController::findKeyStroke(f, Decision(4, 3)).seq());

    EXPECT_EQ(KeySetSeq(">,,>,v"), PuyoController::findKeyStroke(f, Decision(5, 0)).seq());
    EXPECT_EQ(KeySetSeq(">A,,>,v"), PuyoController::findKeyStroke(f, Decision(5, 1)).seq());
    EXPECT_EQ(KeySetSeq(">A,,>,A,,v"), PuyoController::findKeyStroke(f, Decision(5, 2)).seq());
    EXPECT_EQ(KeySetSeq(">B,,>,v"), PuyoController::findKeyStroke(f, Decision(5, 3)).seq());

    EXPECT_EQ(KeySetSeq(">,,>,,>,v"), PuyoController::findKeyStroke(f, Decision(6, 0)).seq());
    EXPECT_EQ(KeySetSeq(">B,,>,,>,B,v"), PuyoController::findKeyStroke(f, Decision(6, 2)).seq());
    EXPECT_EQ(KeySetSeq(">B,,>,,>,v"), PuyoController::findKeyStroke(f, Decision(6, 3)).seq());

    PlainField pf = f.toPlainField();
    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r < 4; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            EXPECT_TRUE(PuyoController::isReachable(f, d));
            KumipuyoMovingState kms(KumipuyoPos::initialPos());
            KeySetSeq kss = PuyoController::findKeyStroke(f, d).seq();
            EXPECT_FALSE(kss.empty());
            for (const auto& ks : kss)
                kms.moveKumipuyo(pf, ks);
            EXPECT_EQ(x, kms.pos.x);
            EXPECT_EQ(r, kms.pos.r);
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
    KumipuyoMovingState kms(KumipuyoPos(3, 12, 0));

    set<Decision> unreachable;
    unreachable.insert(Decision(1, 2));
    unreachable.insert(Decision(6, 2));

    EXPECT_EQ(KeySetSeq("<A,<,<A,<,<B,<,<,<,<B,<,v"), PuyoController::findKeyStroke(f, Decision(1, 0)).seq());
    EXPECT_EQ(KeySetSeq("<A,<,<A,<,<B,<,<,<,v"), PuyoController::findKeyStroke(f, Decision(1, 1)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(1, 2)).seq());

    EXPECT_EQ(KeySetSeq("<,v"), PuyoController::findKeyStroke(f, Decision(2, 0)).seq());
    EXPECT_EQ(KeySetSeq("<A,v"), PuyoController::findKeyStroke(f, Decision(2, 1)).seq());
    EXPECT_EQ(KeySetSeq("<A,,A,v"), PuyoController::findKeyStroke(f, Decision(2, 2)).seq());
    EXPECT_EQ(KeySetSeq("<A,<,<A,<,<A,<,v"), PuyoController::findKeyStroke(f, Decision(2, 3)).seq());

    EXPECT_EQ(KeySetSeq("v"), PuyoController::findKeyStroke(f, Decision(3, 0)).seq());
    EXPECT_EQ(KeySetSeq("A,v"), PuyoController::findKeyStroke(f, Decision(3, 1)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,v"), PuyoController::findKeyStroke(f, Decision(3, 2)).seq());
    EXPECT_EQ(KeySetSeq("B,v"), PuyoController::findKeyStroke(f, Decision(3, 3)).seq());

    EXPECT_EQ(KeySetSeq(">,v"), PuyoController::findKeyStroke(f, Decision(4, 0)).seq());
    EXPECT_EQ(KeySetSeq(">A,v"), PuyoController::findKeyStroke(f, Decision(4, 1)).seq());
    EXPECT_EQ(KeySetSeq(">A,,A,v"), PuyoController::findKeyStroke(f, Decision(4, 2)).seq());
    EXPECT_EQ(KeySetSeq(">B,v"), PuyoController::findKeyStroke(f, Decision(4, 3)).seq());

    EXPECT_EQ(KeySetSeq(">,,>,v"), PuyoController::findKeyStroke(f, Decision(5, 0)).seq());
    EXPECT_EQ(KeySetSeq(">B,>,>,>B,>,>,>,>B,>,v"), PuyoController::findKeyStroke(f, Decision(5, 1)).seq());
    EXPECT_EQ(KeySetSeq(">B,,>,B,,v"), PuyoController::findKeyStroke(f, Decision(5, 2)).seq());
    EXPECT_EQ(KeySetSeq(">B,,>,v"), PuyoController::findKeyStroke(f, Decision(5, 3)).seq());

    EXPECT_EQ(KeySetSeq(">B,>,>,>B,>,>A,>,>A,>,v"), PuyoController::findKeyStroke(f, Decision(6, 0)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(6, 2)).seq());
    EXPECT_EQ(KeySetSeq(">B,>,>,>B,>,>A,>,>,>,v"), PuyoController::findKeyStroke(f, Decision(6, 3)).seq());

    PlainField pf = f.toPlainField();
    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r < 4; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            if (unreachable.count(d)) {
                EXPECT_FALSE(PuyoController::isReachable(f, d));
                EXPECT_TRUE(PuyoController::findKeyStroke(f, d).empty());
                continue;
            }
            EXPECT_TRUE(PuyoController::isReachable(f, d));
            KumipuyoMovingState kms(KumipuyoPos::initialPos());
            KeySetSeq kss = PuyoController::findKeyStroke(f, d).seq();
            EXPECT_FALSE(kss.empty());
            for (const auto& ks : kss)
                kms.moveKumipuyo(pf, ks);
            EXPECT_EQ(x, kms.pos.x);
            EXPECT_EQ(r, kms.pos.r);
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
    KumipuyoMovingState kms(KumipuyoPos(3, 12, 0));

    set<Decision> unreachable;
    unreachable.insert(Decision(2, 2));
    unreachable.insert(Decision(5, 2));

    EXPECT_EQ(KeySetSeq("<A,<,<A,<,<B,<,<,<,<B,<,v"), PuyoController::findKeyStroke(f, Decision(1, 0)).seq());
    EXPECT_EQ(KeySetSeq("<A,<,<A,<,<B,<,<,<,v"), PuyoController::findKeyStroke(f, Decision(1, 1)).seq());
    EXPECT_EQ(KeySetSeq("<A,<,<A,<,<B,<,<,<,<A,<,v"), PuyoController::findKeyStroke(f, Decision(1, 2)).seq());

    EXPECT_EQ(KeySetSeq("A,,A,,B,<,B,v"), PuyoController::findKeyStroke(f, Decision(2, 0)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,,B,<,v"), PuyoController::findKeyStroke(f, Decision(2, 1)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(2, 2)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,,A,<,v"), PuyoController::findKeyStroke(f, Decision(2, 3)).seq());

    EXPECT_EQ(KeySetSeq("v"), PuyoController::findKeyStroke(f, Decision(3, 0)).seq());
    EXPECT_EQ(KeySetSeq("A,v"), PuyoController::findKeyStroke(f, Decision(3, 1)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,v"), PuyoController::findKeyStroke(f, Decision(3, 2)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,,A,v"), PuyoController::findKeyStroke(f, Decision(3, 3)).seq());

    EXPECT_EQ(KeySetSeq(">,v"), PuyoController::findKeyStroke(f, Decision(4, 0)).seq());
    EXPECT_EQ(KeySetSeq(">,B,,B,,B,v"), PuyoController::findKeyStroke(f, Decision(4, 1)).seq());
    EXPECT_EQ(KeySetSeq(">,B,,B,v"), PuyoController::findKeyStroke(f, Decision(4, 2)).seq());
    EXPECT_EQ(KeySetSeq(">,B,v"), PuyoController::findKeyStroke(f, Decision(4, 3)).seq());

    EXPECT_EQ(KeySetSeq(">B,>,>B,>,>B,>,,B,v"), PuyoController::findKeyStroke(f, Decision(5, 0)).seq());
    EXPECT_EQ(KeySetSeq(">B,>,>B,>,>B,>,>,>,v"), PuyoController::findKeyStroke(f, Decision(5, 1)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(5, 2)).seq());
    EXPECT_EQ(KeySetSeq(">B,,>B,,A,>,v"), PuyoController::findKeyStroke(f, Decision(5, 3)).seq());

    EXPECT_EQ(KeySetSeq(">B,>,>B,>,>A,>,>,>,>A,>,v"), PuyoController::findKeyStroke(f, Decision(6, 0)).seq());
    EXPECT_EQ(KeySetSeq(">B,>,>B,>,>A,>,>,>,>B,>,v"), PuyoController::findKeyStroke(f, Decision(6, 2)).seq());
    EXPECT_EQ(KeySetSeq(">B,>,>,>B,>,>A,>,>,>,v"), PuyoController::findKeyStroke(f, Decision(6, 3)).seq());

    PlainField pf = f.toPlainField();
    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r < 4; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            if (unreachable.count(d)) {
                EXPECT_FALSE(PuyoController::isReachable(f, d));
                EXPECT_TRUE(PuyoController::findKeyStroke(f, d).empty());
                continue;
            }
            EXPECT_TRUE(PuyoController::isReachable(f, d));
            KumipuyoMovingState kms(KumipuyoPos::initialPos());
            KeySetSeq kss = PuyoController::findKeyStroke(f, d).seq();
            EXPECT_FALSE(kss.empty());
            for (const auto& ks : kss)
                kms.moveKumipuyo(pf, ks);
            EXPECT_EQ(x, kms.pos.x);
            EXPECT_EQ(r, kms.pos.r);
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
    KumipuyoMovingState kms(KumipuyoPos(3, 12, 0));

    set<Decision> unreachable;
    unreachable.insert(Decision(4, 2));

    EXPECT_EQ(KeySetSeq("<,,<,v"), PuyoController::findKeyStroke(f, Decision(1, 0)).seq());
    EXPECT_EQ(KeySetSeq("<A,,<,v"), PuyoController::findKeyStroke(f, Decision(1, 1)).seq());
    EXPECT_EQ(KeySetSeq("<A,,<,A,v"), PuyoController::findKeyStroke(f, Decision(1, 2)).seq());

    EXPECT_EQ(KeySetSeq("<,v"), PuyoController::findKeyStroke(f, Decision(2, 0)).seq());
    EXPECT_EQ(KeySetSeq("A,v"), PuyoController::findKeyStroke(f, Decision(2, 1)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,v"), PuyoController::findKeyStroke(f, Decision(2, 2)).seq());
    EXPECT_EQ(KeySetSeq("<B,v"), PuyoController::findKeyStroke(f, Decision(2, 3)).seq());

    EXPECT_EQ(KeySetSeq("v"), PuyoController::findKeyStroke(f, Decision(3, 0)).seq());
    EXPECT_EQ(KeySetSeq("B,,B,,B,v"), PuyoController::findKeyStroke(f, Decision(3, 1)).seq());
    EXPECT_EQ(KeySetSeq("B,,B,v"), PuyoController::findKeyStroke(f, Decision(3, 2)).seq());
    EXPECT_EQ(KeySetSeq("B,v"), PuyoController::findKeyStroke(f, Decision(3, 3)).seq());

    EXPECT_EQ(KeySetSeq("B,,B,,B,,>,B,v"), PuyoController::findKeyStroke(f, Decision(4, 0)).seq());
    EXPECT_EQ(KeySetSeq("B,,B,,B,>,v"), PuyoController::findKeyStroke(f, Decision(4, 1)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(4, 2)).seq());
    EXPECT_EQ(KeySetSeq("B,,B,,A,,>,v"), PuyoController::findKeyStroke(f, Decision(4, 3)).seq());

    EXPECT_EQ(KeySetSeq("B,,B,,A,>,A,>,,v"), PuyoController::findKeyStroke(f, Decision(5, 0)).seq());
    EXPECT_EQ(KeySetSeq("B,,B,,B,>,,>,v"), PuyoController::findKeyStroke(f, Decision(5, 1)).seq());
    EXPECT_EQ(KeySetSeq("B,,B,,B,>,,>,A,v"), PuyoController::findKeyStroke(f, Decision(5, 2)).seq());
    EXPECT_EQ(KeySetSeq("B,,B,,A,>,,>,,v"), PuyoController::findKeyStroke(f, Decision(5, 3)).seq());

    EXPECT_EQ(KeySetSeq(">B,>,>B,>,>A,>,>,>,>A,>,v"), PuyoController::findKeyStroke(f, Decision(6, 0)).seq());
    EXPECT_EQ(KeySetSeq(">B,>,>B,>,>A,>,>,>,>B,>,v"), PuyoController::findKeyStroke(f, Decision(6, 2)).seq());
    EXPECT_EQ(KeySetSeq(">B,>,>B,>,>A,>,>,>,>,>,v"), PuyoController::findKeyStroke(f, Decision(6, 3)).seq());

    PlainField pf = f.toPlainField();
    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r < 4; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            if (unreachable.count(d)) {
                EXPECT_FALSE(PuyoController::isReachable(f, d));
                EXPECT_TRUE(PuyoController::findKeyStroke(f, d).empty());
                continue;
            }
            EXPECT_TRUE(PuyoController::isReachable(f, d));
            KumipuyoMovingState kms(KumipuyoPos::initialPos());
            KeySetSeq kss = PuyoController::findKeyStroke(f, d).seq();
            EXPECT_FALSE(kss.empty());
            for (const auto& ks : kss)
                kms.moveKumipuyo(pf, ks);
            EXPECT_EQ(x, kms.pos.x) << d.toString();
            EXPECT_EQ(r, kms.pos.r);
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
    KumipuyoMovingState kms(KumipuyoPos(3, 12, 0));

    set<Decision> unreachable;
    unreachable.insert(Decision(2, 2));
    unreachable.insert(Decision(4, 2));

    EXPECT_EQ(KeySetSeq("<A,<,<A,<,<B,<,<,<,<B,<,v"), PuyoController::findKeyStroke(f, Decision(1, 0)).seq());
    EXPECT_EQ(KeySetSeq("<A,<,<A,<,<B,<,<,<,v"), PuyoController::findKeyStroke(f, Decision(1, 1)).seq());
    EXPECT_EQ(KeySetSeq("<A,<,<A,<,<B,<,<,<,<A,<,v"), PuyoController::findKeyStroke(f, Decision(1, 2)).seq());

    EXPECT_EQ(KeySetSeq("A,,A,,B,<,B,v"), PuyoController::findKeyStroke(f, Decision(2, 0)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,,B,<,v"), PuyoController::findKeyStroke(f, Decision(2, 1)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(2, 2)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,,A,<,v"), PuyoController::findKeyStroke(f, Decision(2, 3)).seq());

    EXPECT_EQ(KeySetSeq("v"), PuyoController::findKeyStroke(f, Decision(3, 0)).seq());
    EXPECT_EQ(KeySetSeq("B,,B,,B,v"), PuyoController::findKeyStroke(f, Decision(3, 1)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,v"), PuyoController::findKeyStroke(f, Decision(3, 2)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,,A,v"), PuyoController::findKeyStroke(f, Decision(3, 3)).seq());

    EXPECT_EQ(KeySetSeq("A,,A,,A,>,A,v"), PuyoController::findKeyStroke(f, Decision(4, 0)).seq());
    EXPECT_EQ(KeySetSeq("B,,B,,B,>,v"), PuyoController::findKeyStroke(f, Decision(4, 1)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(4, 2)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,,A,>,v"), PuyoController::findKeyStroke(f, Decision(4, 3)).seq());

    EXPECT_EQ(KeySetSeq("A,,A,,A,>,A,>,,v"), PuyoController::findKeyStroke(f, Decision(5, 0)).seq());
    EXPECT_EQ(KeySetSeq("B,,B,,B,>,,>,v"), PuyoController::findKeyStroke(f, Decision(5, 1)).seq());
    EXPECT_EQ(KeySetSeq("B,,B,,B,>,,>,A,v"), PuyoController::findKeyStroke(f, Decision(5, 2)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,,A,>,,>,,v"), PuyoController::findKeyStroke(f, Decision(5, 3)).seq());

    EXPECT_EQ(KeySetSeq("A,,A,,A,>,,>,,>,A,v"), PuyoController::findKeyStroke(f, Decision(6, 0)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,,A,>,,>,,>,B,v"), PuyoController::findKeyStroke(f, Decision(6, 2)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,,A,>,,>,,>,v"), PuyoController::findKeyStroke(f, Decision(6, 3)).seq());

    PlainField pf = f.toPlainField();
    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r < 4; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            if (unreachable.count(d)) {
                EXPECT_FALSE(PuyoController::isReachable(f, d));
                EXPECT_TRUE(PuyoController::findKeyStroke(f, d).empty());
                continue;
            }
            EXPECT_TRUE(PuyoController::isReachable(f, d));
            KumipuyoMovingState kms(KumipuyoPos::initialPos());
            KeySetSeq kss = PuyoController::findKeyStroke(f, d).seq();
            EXPECT_FALSE(kss.empty());
            for (const auto& ks : kss)
                kms.moveKumipuyo(pf, ks);
            EXPECT_EQ(x, kms.pos.x);
            EXPECT_EQ(r, kms.pos.r);
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
    KumipuyoMovingState kms(KumipuyoPos(3, 12, 0));

    set<Decision> unreachable;
    unreachable.insert(Decision(2, 2));
    unreachable.insert(Decision(4, 2));

    EXPECT_EQ(KeySetSeq("<A,<,<A,<,<B,<,<,<,<B,<,v"), PuyoController::findKeyStroke(f, Decision(1, 0)).seq());
    EXPECT_EQ(KeySetSeq("<A,<,<A,<,<B,<,<,<,v"), PuyoController::findKeyStroke(f, Decision(1, 1)).seq());
    EXPECT_EQ(KeySetSeq("<A,<,<A,<,<B,<,<,<,<A,<,v"), PuyoController::findKeyStroke(f, Decision(1, 2)).seq());

    EXPECT_EQ(KeySetSeq("A,,A,,B,<,B,v"), PuyoController::findKeyStroke(f, Decision(2, 0)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,,B,<,v"), PuyoController::findKeyStroke(f, Decision(2, 1)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(2, 2)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,,A,<,v"), PuyoController::findKeyStroke(f, Decision(2, 3)).seq());

    EXPECT_EQ(KeySetSeq("v"), PuyoController::findKeyStroke(f, Decision(3, 0)).seq());
    EXPECT_EQ(KeySetSeq("B,,B,,B,v"), PuyoController::findKeyStroke(f, Decision(3, 1)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,v"), PuyoController::findKeyStroke(f, Decision(3, 2)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,,A,v"), PuyoController::findKeyStroke(f, Decision(3, 3)).seq());

    EXPECT_EQ(KeySetSeq("A,,A,,A,>,A,v"), PuyoController::findKeyStroke(f, Decision(4, 0)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,,B,,>,v"), PuyoController::findKeyStroke(f, Decision(4, 1)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(4, 2)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,,A,>,v"), PuyoController::findKeyStroke(f, Decision(4, 3)).seq());

    EXPECT_EQ(KeySetSeq("A,,A,,A,>,A,>,,v"), PuyoController::findKeyStroke(f, Decision(5, 0)).seq());
    EXPECT_EQ(KeySetSeq("B,,B,,A,>,A,>,A,v"), PuyoController::findKeyStroke(f, Decision(5, 1)).seq());
    EXPECT_EQ(KeySetSeq("B,,B,,A,>,A,>,A,,A,v"), PuyoController::findKeyStroke(f, Decision(5, 2)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,,A,>,,>,,v"), PuyoController::findKeyStroke(f, Decision(5, 3)).seq());

    EXPECT_EQ(KeySetSeq("A,,A,,A,>,,>,,>,A,v"), PuyoController::findKeyStroke(f, Decision(6, 0)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,,A,>,,>,,>,B,v"), PuyoController::findKeyStroke(f, Decision(6, 2)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,,A,>,,>,,>,v"), PuyoController::findKeyStroke(f, Decision(6, 3)).seq());

    PlainField pf = f.toPlainField();
    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r < 4; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            if (unreachable.count(d)) {
                EXPECT_FALSE(PuyoController::isReachable(f, d));
                EXPECT_TRUE(PuyoController::findKeyStroke(f, d).empty());
                continue;
            }
            EXPECT_TRUE(PuyoController::isReachable(f, d)) << d.toString();
            KumipuyoMovingState kms(KumipuyoPos::initialPos());
            KeySetSeq kss = PuyoController::findKeyStroke(f, d).seq();
            EXPECT_FALSE(kss.empty()) << d.toString();
            for (const auto& ks : kss)
                kms.moveKumipuyo(pf, ks);
            EXPECT_EQ(x, kms.pos.x) << d.toString();
            EXPECT_EQ(r, kms.pos.r) << d.toString();
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
    KumipuyoMovingState kms(KumipuyoPos(3, 12, 0));

    set<Decision> reachable;
    reachable.insert(Decision(3, 0));
    reachable.insert(Decision(3, 2));

    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(1, 0)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(1, 1)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(1, 2)).seq());

    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(2, 0)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(2, 1)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(2, 2)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(2, 3)).seq());

    EXPECT_EQ(KeySetSeq("v"), PuyoController::findKeyStroke(f, Decision(3, 0)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(3, 1)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,v"), PuyoController::findKeyStroke(f, Decision(3, 2)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(3, 3)).seq());

    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(4, 0)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(4, 1)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(4, 2)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(4, 3)).seq());

    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(5, 0)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(5, 1)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(5, 2)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(5, 3)).seq());

    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(6, 0)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(6, 2)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(6, 3)).seq());

    PlainField pf = f.toPlainField();
    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r < 4; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            if (!reachable.count(d)) {
                EXPECT_FALSE(PuyoController::isReachable(f, d))
                    << d.toString() << " " << PuyoController::findKeyStroke(f, d).seq().toString();
                EXPECT_TRUE(PuyoController::findKeyStroke(f, d).empty()) << d.toString();
                continue;
            }
            EXPECT_TRUE(PuyoController::isReachable(f, d));
            KumipuyoMovingState kms(KumipuyoPos::initialPos());
            KeySetSeq kss = PuyoController::findKeyStroke(f, d).seq();
            EXPECT_FALSE(kss.empty());
            for (const auto& ks : kss)
                kms.moveKumipuyo(pf, ks);
            EXPECT_EQ(x, kms.pos.x);
            EXPECT_EQ(r, kms.pos.r);
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
    KumipuyoMovingState kms(KumipuyoPos(3, 12, 0));

    set<Decision> reachable;
    reachable.insert(Decision(2, 0));
    reachable.insert(Decision(2, 1));
    reachable.insert(Decision(3, 0));
    reachable.insert(Decision(3, 1));
    reachable.insert(Decision(3, 2));
    reachable.insert(Decision(3, 3));
    reachable.insert(Decision(4, 0));
    reachable.insert(Decision(4, 3));

    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(1, 0)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(1, 1)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(1, 2)).seq());

    EXPECT_EQ(KeySetSeq("A,,A,,B,<,B,v"), PuyoController::findKeyStroke(f, Decision(2, 0)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,,B,<,v"), PuyoController::findKeyStroke(f, Decision(2, 1)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(2, 2)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(2, 3)).seq());

    EXPECT_EQ(KeySetSeq("v"), PuyoController::findKeyStroke(f, Decision(3, 0)).seq());
    EXPECT_EQ(KeySetSeq("B,,B,,B,v"), PuyoController::findKeyStroke(f, Decision(3, 1)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,v"), PuyoController::findKeyStroke(f, Decision(3, 2)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,,A,v"), PuyoController::findKeyStroke(f, Decision(3, 3)).seq());

    EXPECT_EQ(KeySetSeq("A,,A,,A,>,A,v"), PuyoController::findKeyStroke(f, Decision(4, 0)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(4, 1)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(4, 2)).seq());
    EXPECT_EQ(KeySetSeq("A,,A,,A,>,v"), PuyoController::findKeyStroke(f, Decision(4, 3)).seq());

    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(5, 0)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(5, 1)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(5, 2)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(5, 3)).seq());

    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(6, 0)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(6, 2)).seq());
    EXPECT_EQ(KeySetSeq(), PuyoController::findKeyStroke(f, Decision(6, 3)).seq());

    PlainField pf = f.toPlainField();
    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r < 4; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            if (!reachable.count(d)) {
                EXPECT_FALSE(PuyoController::isReachable(f, d));
                EXPECT_TRUE(PuyoController::findKeyStroke(f, d).empty());
                continue;
            }
            EXPECT_TRUE(PuyoController::isReachable(f, d));
            KumipuyoMovingState kms(KumipuyoPos::initialPos());
            KeySetSeq kss = PuyoController::findKeyStroke(f, d).seq();
            EXPECT_FALSE(kss.empty());
            for (const auto& ks : kss)
                kms.moveKumipuyo(pf, ks);
            EXPECT_EQ(x, kms.pos.x);
            EXPECT_EQ(r, kms.pos.r);
        }
    }
}

TEST(PuyoControllerTest, findKeyStrokeHigherExhaustive)
{
    CoreField f(
        "OOOOOO" // 10
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    for (int i = 0; i < 4 * 4 * 4 * 4 * 4; ++i) {
        int heights[] = {
            10,
            10 + ((i >> 0) & 3),
            10 + ((i >> 2) & 3),
            10,
            10 + ((i >> 4) & 3),
            10 + ((i >> 6) & 3),
            10 + ((i >> 8) & 3),
        };

        for (int x = 1; x <= 6; ++x) {
            int h = heights[x];
            while (f.height(x) > h)
                f.removePuyoFrom(x);
            while (f.height(x) < h)
                f.dropPuyoOn(x, PuyoColor::OJAMA);
            ASSERT_TRUE(f.height(x) == h);
        }

        PlainField pf = f.toPlainField();

        for (int x = 1; x <= 6; ++x) {
            for (int r = 0; r < 4; ++r) {
                Decision d(x, r);
                if (!d.isValid())
                    continue;
                bool reachable = PuyoController::isReachable(f, d);

                KumipuyoMovingState kms(KumipuyoPos::initialPos());
                KeySetSeq kss = PuyoController::findKeyStroke(f, d).seq();
                if (!reachable) {
                    EXPECT_TRUE(kss.empty()) << f.toDebugString() << '\n' << d.toString();
                    continue;
                }

                EXPECT_FALSE(kss.empty());
                for (const auto& ks : kss)
                    kms.moveKumipuyo(pf, ks);
                EXPECT_EQ(x, kms.pos.x) << pf.toDebugString() << '\n' << d.toString() << ' ' << kss.toString();
                EXPECT_EQ(r, kms.pos.r) << pf.toDebugString() << '\n' << d.toString() << ' ' << kss.toString();
            }
        }
    }
}

TEST(PuyoControllerTest, reachable1)
{
    CoreField f(
        "     O" // 14
        "     O"
        "    OO" // 12
        "OO OOO"
        "OO OOO"
        "OO OOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    set<Decision> unreachable;
    unreachable.insert(Decision(5, 1));
    unreachable.insert(Decision(5, 2));
    unreachable.insert(Decision(6, 0));
    unreachable.insert(Decision(6, 2));
    unreachable.insert(Decision(6, 3));

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r <= 3; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            if (unreachable.count(d))
                EXPECT_FALSE(PuyoController::isReachable(f, d)) << d.toString();
            else
                EXPECT_TRUE(PuyoController::isReachable(f, d)) << d.toString();
        }
    }
}

TEST(PuyoControllerTest, reachable2)
{
    CoreField f(
        "    O " // 14
        "    O "
        "    OO" // 12
        "OO OOO"
        "OO OOO"
        "OO OOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    set<Decision> unreachable;
    unreachable.insert(Decision(4, 1));
    unreachable.insert(Decision(5, 0));
    unreachable.insert(Decision(5, 1));
    unreachable.insert(Decision(5, 2));
    unreachable.insert(Decision(5, 3));
    unreachable.insert(Decision(6, 0));
    unreachable.insert(Decision(6, 2));
    unreachable.insert(Decision(6, 3));

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r <= 3; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            if (unreachable.count(d))
                EXPECT_FALSE(PuyoController::isReachable(f, d)) << d.toString();
            else
                EXPECT_TRUE(PuyoController::isReachable(f, d)) << d.toString();
        }
    }
}

TEST(PuyoControllerTest, reachable3)
{
    CoreField f(
        "   O  " // 14
        "   O  "
        "   O  " // 12
        "OO OOO"
        "OO OOO"
        "OO OOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    set<Decision> unreachable;
    unreachable.insert(Decision(3, 1));
    unreachable.insert(Decision(4, 0));
    unreachable.insert(Decision(4, 1));
    unreachable.insert(Decision(4, 2));
    unreachable.insert(Decision(4, 3));
    unreachable.insert(Decision(5, 0));
    unreachable.insert(Decision(5, 1));
    unreachable.insert(Decision(5, 2));
    unreachable.insert(Decision(5, 3));
    unreachable.insert(Decision(6, 0));
    unreachable.insert(Decision(6, 2));
    unreachable.insert(Decision(6, 3));

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r <= 3; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            if (unreachable.count(d))
                EXPECT_FALSE(PuyoController::isReachable(f, d)) << d.toString();
            else
                EXPECT_TRUE(PuyoController::isReachable(f, d)) << d.toString();
        }
    }
}

TEST(PuyoControllerTest, reachable4)
{
    CoreField f(
        " O    " // 14
        " O    "
        " O    " // 12
        "OO OOO"
        "OO OOO"
        "OO OOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    set<Decision> unreachable;
    unreachable.insert(Decision(1, 0));
    unreachable.insert(Decision(1, 1));
    unreachable.insert(Decision(1, 2));
    unreachable.insert(Decision(2, 0));
    unreachable.insert(Decision(2, 1));
    unreachable.insert(Decision(2, 2));
    unreachable.insert(Decision(2, 3));
    unreachable.insert(Decision(3, 3));

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r <= 3; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            if (unreachable.count(d))
                EXPECT_FALSE(PuyoController::isReachable(f, d)) << d.toString();
            else
                EXPECT_TRUE(PuyoController::isReachable(f, d)) << d.toString();
        }
    }
}

TEST(PuyoControllerTest, reachable6)
{
    CoreField f(
        "O     " // 14
        "O     "
        "O     " // 12
        "OO OOO"
        "OO OOO"
        "OO OOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO");

    set<Decision> unreachable;
    unreachable.insert(Decision(1, 0));
    unreachable.insert(Decision(1, 1));
    unreachable.insert(Decision(1, 2));
    unreachable.insert(Decision(2, 3));

    for (int x = 1; x <= 6; ++x) {
        for (int r = 0; r <= 3; ++r) {
            Decision d(x, r);
            if (!d.isValid())
                continue;
            if (unreachable.count(d))
                EXPECT_FALSE(PuyoController::isReachable(f, d)) << d.toString();
            else
                EXPECT_TRUE(PuyoController::isReachable(f, d)) << d.toString();
        }
    }
}
