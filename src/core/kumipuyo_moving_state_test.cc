#include "core/kumipuyo_moving_state.h"

#include <gtest/gtest.h>

#include "core/core_field.h"
#include "core/key.h"
#include "core/key_set.h"

using namespace std;

TEST(KumipuyoMovingStateTest, moveKumipuyoWithOnlyArrowKey)
{
    CoreField f;
    KumipuyoMovingState kms(KumipuyoPos(3, 12, 0));
    bool downAccepted = false;

    kms.moveKumipuyo(f, KeySet(Key::RIGHT), &downAccepted);
    EXPECT_EQ(KumipuyoPos(4, 12, 0), kms.pos);
    EXPECT_EQ(0, kms.restFramesTurnProhibited);
    EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
    EXPECT_EQ(FRAMES_FREE_FALL - 1, kms.restFramesForFreefall);
    EXPECT_FALSE(kms.grounded);

    kms.moveKumipuyo(f, KeySet(), &downAccepted);
    kms.moveKumipuyo(f, KeySet(Key::LEFT), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 12, 0), kms.pos);
    EXPECT_EQ(0, kms.restFramesTurnProhibited);
    EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
    EXPECT_EQ(FRAMES_FREE_FALL - 3, kms.restFramesForFreefall);
    EXPECT_FALSE(kms.grounded);

    // TODO(mayah): Should we accept Key::DOWN here?
    kms.moveKumipuyo(f, KeySet(), &downAccepted);
    kms.moveKumipuyo(f, KeySet(Key::DOWN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 12, 0), kms.pos);
    EXPECT_EQ(0, kms.restFramesTurnProhibited);
    EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
    EXPECT_EQ(0, kms.restFramesForFreefall);
    EXPECT_FALSE(kms.grounded);
    EXPECT_TRUE(downAccepted);

    downAccepted = false;
    kms.moveKumipuyo(f, KeySet(Key::DOWN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 11, 0), kms.pos);
    EXPECT_EQ(0, kms.restFramesTurnProhibited);
    EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
    EXPECT_EQ(0, kms.restFramesForFreefall);
    EXPECT_FALSE(kms.grounded);
    EXPECT_TRUE(downAccepted);

    kms.pos = KumipuyoPos(3, 1, 0);
    kms.grounding = false;
    kms.restFramesForFreefall = FRAMES_FREE_FALL;

    downAccepted = false;
    kms.moveKumipuyo(f, KeySet(Key::DOWN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 1, 0), kms.pos);
    EXPECT_EQ(0, kms.restFramesTurnProhibited);
    EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
    EXPECT_EQ(FRAMES_FREE_FALL, kms.restFramesForFreefall);
    EXPECT_TRUE(kms.grounding);
    EXPECT_FALSE(kms.grounded);
    EXPECT_TRUE(downAccepted);

    downAccepted = false;
    kms.moveKumipuyo(f, KeySet(Key::DOWN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 1, 0), kms.pos);
    EXPECT_EQ(0, kms.restFramesTurnProhibited);
    EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
    EXPECT_EQ(0, kms.restFramesForFreefall);
    EXPECT_TRUE(kms.grounded);
    EXPECT_TRUE(downAccepted);
}

TEST(KumipuyoMovingStateTest, moveKumipuyoWithOnlyArrowKey_Right)
{
    CoreField f;
    bool downAccepted = false;

    {
        KumipuyoMovingState kms(KumipuyoPos(3, 12, 0));
        kms.moveKumipuyo(f, KeySet(Key::RIGHT), &downAccepted);
        EXPECT_EQ(KumipuyoPos(4, 12, 0), kms.pos);
        EXPECT_EQ(0, kms.restFramesTurnProhibited);
        EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
        EXPECT_EQ(FRAMES_FREE_FALL - 1, kms.restFramesForFreefall);
        EXPECT_FALSE(kms.grounded);
    }
    {
        KumipuyoMovingState kms(KumipuyoPos(3, 12, 1));
        kms.moveKumipuyo(f, KeySet(Key::RIGHT), &downAccepted);
        EXPECT_EQ(KumipuyoPos(4, 12, 1), kms.pos);
        EXPECT_EQ(0, kms.restFramesTurnProhibited);
        EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
        EXPECT_EQ(FRAMES_FREE_FALL - 1, kms.restFramesForFreefall);
        EXPECT_FALSE(kms.grounded);
    }
    {
        KumipuyoMovingState kms(KumipuyoPos(3, 11, 2));
        kms.moveKumipuyo(f, KeySet(Key::RIGHT), &downAccepted);
        EXPECT_EQ(KumipuyoPos(4, 11, 2), kms.pos);
        EXPECT_EQ(0, kms.restFramesTurnProhibited);
        EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
        EXPECT_EQ(FRAMES_FREE_FALL - 1, kms.restFramesForFreefall);
        EXPECT_FALSE(kms.grounded);
    }
    {
        KumipuyoMovingState kms(KumipuyoPos(3, 12, 3));
        kms.moveKumipuyo(f, KeySet(Key::RIGHT), &downAccepted);
        EXPECT_EQ(KumipuyoPos(4, 12, 3), kms.pos);
        EXPECT_EQ(0, kms.restFramesTurnProhibited);
        EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
        EXPECT_EQ(FRAMES_FREE_FALL - 1, kms.restFramesForFreefall);
        EXPECT_FALSE(kms.grounded);
    }
}

TEST(KumipuyoMovingStateTest, moveKumipuyoWithOnlyArrowKey_Left)
{
    CoreField f;
    bool downAccepted = false;

    {
        KumipuyoMovingState kms(KumipuyoPos(3, 12, 0));
        kms.moveKumipuyo(f, KeySet(Key::LEFT), &downAccepted);
        EXPECT_EQ(KumipuyoPos(2, 12, 0), kms.pos);
        EXPECT_EQ(0, kms.restFramesTurnProhibited);
        EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
        EXPECT_EQ(FRAMES_FREE_FALL - 1, kms.restFramesForFreefall);
        EXPECT_FALSE(kms.grounded);
    }
    {
        KumipuyoMovingState kms(KumipuyoPos(3, 12, 1));
        kms.moveKumipuyo(f, KeySet(Key::LEFT), &downAccepted);
        EXPECT_EQ(KumipuyoPos(2, 12, 1), kms.pos);
        EXPECT_EQ(0, kms.restFramesTurnProhibited);
        EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
        EXPECT_EQ(FRAMES_FREE_FALL - 1, kms.restFramesForFreefall);
        EXPECT_FALSE(kms.grounded);
    }
    {
        KumipuyoMovingState kms(KumipuyoPos(3, 11, 2));
        kms.moveKumipuyo(f, KeySet(Key::LEFT), &downAccepted);
        EXPECT_EQ(KumipuyoPos(2, 11, 2), kms.pos);
        EXPECT_EQ(0, kms.restFramesTurnProhibited);
        EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
        EXPECT_EQ(FRAMES_FREE_FALL - 1, kms.restFramesForFreefall);
        EXPECT_FALSE(kms.grounded);
    }
    {
        KumipuyoMovingState kms(KumipuyoPos(3, 12, 3));
        kms.moveKumipuyo(f, KeySet(Key::LEFT), &downAccepted);
        EXPECT_EQ(KumipuyoPos(2, 12, 3), kms.pos);
        EXPECT_EQ(0, kms.restFramesTurnProhibited);
        EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
        EXPECT_EQ(FRAMES_FREE_FALL - 1, kms.restFramesForFreefall);
        EXPECT_FALSE(kms.grounded);
    }
}

TEST(KumipuyoMovingStateTest, moveKumipuyoFreefall)
{
    CoreField f;
    KumipuyoMovingState kms(KumipuyoPos(3, 12, 0));
    kms.restFramesForFreefall = 2;
    bool downAccepted = false;

    kms.moveKumipuyo(f, KeySet(), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 12, 0), kms.pos);
    EXPECT_EQ(0, kms.restFramesTurnProhibited);
    EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
    EXPECT_EQ(1, kms.restFramesForFreefall);
    EXPECT_FALSE(kms.grounded);
    EXPECT_FALSE(downAccepted);

    kms.moveKumipuyo(f, KeySet(), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 11, 0), kms.pos);
    EXPECT_EQ(0, kms.restFramesTurnProhibited);
    EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
    EXPECT_EQ(FRAMES_FREE_FALL, kms.restFramesForFreefall);
    EXPECT_FALSE(kms.grounded);
    EXPECT_FALSE(downAccepted);

    kms.pos = KumipuyoPos(3, 1, 0);
    kms.grounding = false;
    kms.restFramesForFreefall = FRAMES_FREE_FALL / 2 + 1;

    kms.moveKumipuyo(f, KeySet(), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 1, 0), kms.pos);
    EXPECT_EQ(0, kms.restFramesTurnProhibited);
    EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
    EXPECT_EQ(FRAMES_FREE_FALL, kms.restFramesForFreefall);
    EXPECT_FALSE(kms.grounded);
    EXPECT_TRUE(kms.grounding);
    EXPECT_FALSE(downAccepted);

    kms.restFramesForFreefall = 1;

    kms.moveKumipuyo(f, KeySet(), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 1, 0), kms.pos);
    EXPECT_EQ(0, kms.restFramesTurnProhibited);
    EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
    EXPECT_TRUE(kms.grounded);
    EXPECT_FALSE(downAccepted);
}

TEST(KumipuyoMovingStateTest, moveKumipuyoWithTurnKey)
{
    CoreField f;
    KumipuyoMovingState kms(KumipuyoPos(3, 12, 0));
    bool downAccepted = false;

    kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 12, 1), kms.pos);
    EXPECT_EQ(KumipuyoMovingState::FRAMES_CONTINUOUS_TURN_PROHIBITED, kms.restFramesTurnProhibited);
    EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
    EXPECT_EQ(FRAMES_FREE_FALL - 1, kms.restFramesForFreefall);
    EXPECT_FALSE(kms.grounded);
    EXPECT_FALSE(downAccepted);

    // RIGHT_TURN should be ignored during kms.resetFrames.TurnProhibited > 0.
    kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 12, 1), kms.pos);
    EXPECT_EQ(KumipuyoMovingState::FRAMES_CONTINUOUS_TURN_PROHIBITED - 1, kms.restFramesTurnProhibited);
    EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
    EXPECT_EQ(FRAMES_FREE_FALL - 2, kms.restFramesForFreefall);
    EXPECT_FALSE(kms.grounded);
    EXPECT_FALSE(downAccepted);

    while (kms.restFramesTurnProhibited > 0) {
        kms.moveKumipuyo(f, KeySet(), &downAccepted);
    }

    kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 12, 2), kms.pos);
}

TEST(KumipuyoMovingStateTest, moveKumipuyoMultipleKeys)
{
    CoreField f;
    KumipuyoMovingState kms(KumipuyoPos(1, 12, 0));
    bool downAccepted = false;

    kms.moveKumipuyo(f, KeySet(Key::RIGHT, Key::LEFT_TURN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 12, 3), kms.pos);
    EXPECT_EQ(KumipuyoMovingState::FRAMES_CONTINUOUS_TURN_PROHIBITED, kms.restFramesTurnProhibited);
    EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
    EXPECT_EQ(FRAMES_FREE_FALL - 1, kms.restFramesForFreefall);
    EXPECT_FALSE(kms.grounded);
    EXPECT_FALSE(downAccepted);
}

TEST(KumipuyoMovingStateTest, wontGrounding)
{
    CoreField f;
    KumipuyoMovingState kms(KumipuyoPos(3, 2, 1));
    kms.restFramesForFreefall = FRAMES_FREE_FALL;
    bool downAccepted = false;

    EXPECT_EQ(0, kms.numGrounded);
    EXPECT_FALSE(kms.grounding);

    // In this case, it won't move to the grounding state.
    kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);

    EXPECT_EQ(1, kms.pos.childY());
    EXPECT_EQ(0, kms.numGrounded);
    EXPECT_FALSE(kms.grounding);
}

TEST(KumipuyoMovingStateTest, grounding)
{
    CoreField f;
    KumipuyoMovingState kms(KumipuyoPos(3, 2, 1));
    kms.restFramesForFreefall = FRAMES_FREE_FALL / 2;
    bool downAccepted = false;

    EXPECT_EQ(0, kms.numGrounded);
    EXPECT_FALSE(kms.grounding);

    // In this case, it should move to the grounding state.
    kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);

    EXPECT_EQ(1, kms.pos.childY());
    EXPECT_EQ(1, kms.numGrounded);
    EXPECT_TRUE(kms.grounding);
}

TEST(KumipuyoMovingStateTest, fixAfterEightGrouding)
{
    CoreField f;
    KumipuyoMovingState kms(KumipuyoPos(3, 2, 1));
    bool downAccepted = false;

    kms.restFramesForFreefall = FRAMES_FREE_FALL / 2;

    EXPECT_EQ(0, kms.numGrounded);
    EXPECT_FALSE(kms.grounding);

    for (int i = 0; i < 7; ++i) {
        kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);
        EXPECT_EQ(1, kms.pos.childY());
        EXPECT_EQ(i + 1, kms.numGrounded) << i;
        EXPECT_TRUE(kms.grounding);

        kms.moveKumipuyo(f, KeySet(), &downAccepted);
        EXPECT_EQ(1, kms.pos.childY());
        EXPECT_EQ(i + 1, kms.numGrounded) << i;
        EXPECT_TRUE(kms.grounding);

        kms.moveKumipuyo(f, KeySet(Key::LEFT_TURN), &downAccepted);
        EXPECT_EQ(2, kms.pos.childY());
        EXPECT_EQ(i + 1, kms.numGrounded) << i;
        EXPECT_FALSE(kms.grounding);

        kms.moveKumipuyo(f, KeySet(), &downAccepted);
        EXPECT_EQ(2, kms.pos.childY());
        EXPECT_EQ(i + 1, kms.numGrounded) << i;
        EXPECT_FALSE(kms.grounding);
    }

    // After 8th grounding, kumipuyo should be fixed.
    kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);
    EXPECT_EQ(1, kms.pos.childY());
    EXPECT_EQ(8, kms.numGrounded);
    EXPECT_TRUE(kms.grounded);
}

TEST(KumipuyoMovingStateTest, moveKumipuyoWithLiftingAxis_RightTurn)
{
    CoreField f;
    bool downAccepted = false;

    KumipuyoMovingState kms(KumipuyoPos(3, 1, 1));
    kms.grounding = true;

    kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 2, 2), kms.pos);
    kms.moveKumipuyo(f, KeySet(), &downAccepted);
    kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 2, 3), kms.pos);
    kms.moveKumipuyo(f, KeySet(), &downAccepted);
    kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 2, 0), kms.pos);
    kms.moveKumipuyo(f, KeySet(), &downAccepted);
    kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 1, 1), kms.pos);
    kms.moveKumipuyo(f, KeySet(), &downAccepted);
    kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 2, 2), kms.pos);
}

TEST(KumipuyoMovingStateTest, moveKumipuyoWithLiftingAxis_LeftTurn)
{
    CoreField f;
    bool downAccepted = false;

    KumipuyoMovingState kms(KumipuyoPos(3, 1, 3));
    kms.grounding = true;

    kms.moveKumipuyo(f, KeySet(Key::LEFT_TURN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 2, 2), kms.pos);
    kms.moveKumipuyo(f, KeySet(), &downAccepted);
    kms.moveKumipuyo(f, KeySet(Key::LEFT_TURN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 2, 1), kms.pos);
    kms.moveKumipuyo(f, KeySet(), &downAccepted);
    kms.moveKumipuyo(f, KeySet(Key::LEFT_TURN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 2, 0), kms.pos);
    kms.moveKumipuyo(f, KeySet(), &downAccepted);
    kms.moveKumipuyo(f, KeySet(Key::LEFT_TURN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 1, 3), kms.pos);
    kms.moveKumipuyo(f, KeySet(), &downAccepted);
    kms.moveKumipuyo(f, KeySet(Key::LEFT_TURN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 2, 2), kms.pos);
}

TEST(KumipuyoMovingStateTest, moveKumipuyoWithLiftingAxis12)
{
    CoreField f(
        "OOOOOO" // 11
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

    bool downAccepted = false;

    KumipuyoMovingState kms(KumipuyoPos(3, 12, 1));
    kms.grounding = true;

    kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 13, 2), kms.pos);
    kms.moveKumipuyo(f, KeySet(), &downAccepted);
    kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 13, 3), kms.pos);
    kms.moveKumipuyo(f, KeySet(), &downAccepted);
    kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 13, 0), kms.pos);
    kms.moveKumipuyo(f, KeySet(), &downAccepted);
    kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 12, 1), kms.pos);
    kms.moveKumipuyo(f, KeySet(), &downAccepted);
    kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 13, 2), kms.pos);
}

TEST(KumipuyoMovingStateTest, moveKumipuyoWithLiftingAxis13)
{
    CoreField f(
        "   OOO" // 12
        "OOOOOO" // 11
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 8
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        "OOOOOO" // 4
        "OOOOOO"
        "OOOOOO"
        "OOOOOO"
        );
    bool downAccepted = false;

    {
        KumipuyoMovingState kms(KumipuyoPos(5, 13, 1));
        kms.grounding = true;

        kms.moveKumipuyo(f, KeySet(Key::LEFT_TURN), &downAccepted);
        EXPECT_EQ(KumipuyoPos(5, 13, 0), kms.pos);
    }

    {
        // Axis cannot be moved to the 14th line.
        KumipuyoMovingState kms(KumipuyoPos(5, 13, 1));
        kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);
        EXPECT_EQ(KumipuyoPos(5, 13, 1), kms.pos);
    }
}

TEST(KumipuyoMovingStateTest, moveKumipuyoQuickTurn)
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
    bool downAccepted = false;

    kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 12, 0), kms.pos);
    EXPECT_EQ(KumipuyoMovingState::FRAMES_CONTINUOUS_TURN_PROHIBITED, kms.restFramesTurnProhibited);
    EXPECT_EQ(FRAMES_QUICKTURN, kms.restFramesToAcceptQuickTurn);
    EXPECT_EQ(FRAMES_FREE_FALL - 1, kms.restFramesForFreefall);
    EXPECT_FALSE(kms.grounded);
    EXPECT_FALSE(downAccepted);

    kms.moveKumipuyo(f, KeySet(), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 12, 0), kms.pos);
    EXPECT_EQ(KumipuyoMovingState::FRAMES_CONTINUOUS_TURN_PROHIBITED - 1, kms.restFramesTurnProhibited);
    EXPECT_EQ(FRAMES_QUICKTURN - 1, kms.restFramesToAcceptQuickTurn);
    EXPECT_EQ(FRAMES_FREE_FALL - 2, kms.restFramesForFreefall);
    EXPECT_FALSE(kms.grounded);
    EXPECT_FALSE(downAccepted);

    kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(3, 13, 2), kms.pos);
    EXPECT_EQ(KumipuyoMovingState::FRAMES_CONTINUOUS_TURN_PROHIBITED, kms.restFramesTurnProhibited);
    EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
    EXPECT_EQ(FRAMES_FREE_FALL / 2, kms.restFramesForFreefall); // After lifted, frames free fall should be an initial value / 2.
    EXPECT_FALSE(kms.grounded);
    EXPECT_FALSE(downAccepted);

    kms.moveKumipuyo(f, KeySet(), &downAccepted);
    kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);
    kms.moveKumipuyo(f, KeySet(Key::RIGHT), &downAccepted);
    kms.moveKumipuyo(f, KeySet(Key::RIGHT_TURN), &downAccepted);
    kms.moveKumipuyo(f, KeySet(Key::DOWN), &downAccepted);
    EXPECT_EQ(KumipuyoPos(4, 13, 0), kms.pos);
}

TEST(KumipuyoMovingStateTest, moveForWii)
{
    CoreField f;
    KumipuyoMovingState kms(KumipuyoPos(3, 12, 0));
    bool downAccepted = false;

    kms.moveKumipuyo(f, KeySet(Key::RIGHT), &downAccepted);
    EXPECT_EQ(KumipuyoPos(4, 12, 0), kms.pos);
    EXPECT_EQ(0, kms.restFramesTurnProhibited);
    EXPECT_EQ(1, kms.restFramesArrowProhibited);
    EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
    EXPECT_EQ(FRAMES_FREE_FALL - 1, kms.restFramesForFreefall);
    EXPECT_FALSE(kms.grounded);

    // This should be ignored.
    kms.moveKumipuyo(f, KeySet(Key::RIGHT), &downAccepted);
    EXPECT_EQ(KumipuyoPos(4, 12, 0), kms.pos);
    EXPECT_EQ(0, kms.restFramesTurnProhibited);
    EXPECT_EQ(0, kms.restFramesArrowProhibited);
    EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
    EXPECT_EQ(FRAMES_FREE_FALL - 2, kms.restFramesForFreefall);
    EXPECT_FALSE(kms.grounded);

    kms.moveKumipuyo(f, KeySet(Key::RIGHT), &downAccepted);
    EXPECT_EQ(KumipuyoPos(5, 12, 0), kms.pos);
    EXPECT_EQ(0, kms.restFramesTurnProhibited);
    EXPECT_EQ(1, kms.restFramesArrowProhibited);
    EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
    EXPECT_EQ(FRAMES_FREE_FALL - 3, kms.restFramesForFreefall);
    EXPECT_FALSE(kms.grounded);

    // This should be ignored.
    kms.moveKumipuyo(f, KeySet(Key::LEFT), &downAccepted);
    EXPECT_EQ(KumipuyoPos(5, 12, 0), kms.pos);
    EXPECT_EQ(0, kms.restFramesTurnProhibited);
    EXPECT_EQ(0, kms.restFramesArrowProhibited);
    EXPECT_EQ(0, kms.restFramesToAcceptQuickTurn);
    EXPECT_EQ(FRAMES_FREE_FALL - 4, kms.restFramesForFreefall);
    EXPECT_FALSE(kms.grounded);
}
