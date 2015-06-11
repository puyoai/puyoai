#include "duel/field_realtime.h"

#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "core/core_field.h"
#include "core/kumipuyo.h"
#include "core/plain_field.h"
#include "core/puyo_controller.h"
#include "duel/field_realtime.h"
#include "duel/frame_context.h"

using namespace std;

class FieldRealtimeTest : public ::testing::Test {
protected:
    virtual void SetUp()
    {
        string sequence = "RRGGBBYY";
        f_.reset(new FieldRealtime(0, sequence));
    }

    virtual void TearDown()
    {
        f_.reset();
    }

    unique_ptr<FieldRealtime> f_;
};

TEST_F(FieldRealtimeTest, stateWithoutOjama)
{
    f_->skipLevelSelect();

    // Waiting next.
    for (int i = 0; i < FRAMES_PREPARING_NEXT; ++i) {
        FrameContext context;
        f_->playOneFrame(KeySet(Key::DOWN), &context);
        EXPECT_EQ(FieldRealtime::SimulationState::STATE_PREPARING_NEXT, f_->simulationState());
    }

    // It needs 12 frames to reach the bottom.
    for (int i = 0; i < FRAMES_TO_DROP_FAST[12]; ++i) {
        FrameContext context;
        f_->playOneFrame(KeySet(Key::DOWN), &context);
        EXPECT_EQ(FieldRealtime::SimulationState::STATE_PLAYABLE, f_->simulationState());
    }

    // Then, 1 frame dropping. (Even without dropping, AC puyo 2 has 1 frame here.
    // Actually it might have to be counted as PLAYABLE, though.
    {
        FrameContext context;
        f_->playOneFrame(KeySet(Key::DOWN), &context);
        EXPECT_EQ(FieldRealtime::SimulationState::STATE_DROPPING, f_->simulationState());
    }

    // Then, 10 frames ground animation.
    for (int i = 0; i < FRAMES_GROUNDING; ++i) {
        FrameContext context;
        f_->playOneFrame(KeySet(Key::DOWN), &context);
        EXPECT_EQ(FieldRealtime::SimulationState::STATE_GROUNDING, f_->simulationState());
    }

    // Then, preparing next.
    FrameContext context;
    f_->playOneFrame(KeySet(Key::DOWN), &context);
    EXPECT_EQ(FieldRealtime::SimulationState::STATE_PREPARING_NEXT, f_->simulationState());

    // Here, score must be 13. 12 frames dropping + 1 frame grounding.
    EXPECT_EQ(13, f_->score());
}

TEST_F(FieldRealtimeTest, zenkeshi)
{
    f_->forceSetField(PlainField("  RR  "));
    f_->skipLevelSelect();

    // Waiting next (6) + reaching bottom (11) + dropping (1) + grounding (10)
    for (int i = 0; i < 6 + 11 + 1 + 10; ++i) {
        FrameContext context;
        f_->playOneFrame(KeySet(Key::DOWN), &context);
        cout << (int)f_->simulationState() << endl;
        EXPECT_FALSE(f_->hasZenkeshi());
    }

    FrameContext context;
    f_->playOneFrame(KeySet(Key::DOWN), &context);
    EXPECT_TRUE(f_->hasZenkeshi());
    EXPECT_EQ(0, context.numSentOjama());

    for (int i = 0; i < 24; ++i) {
        FrameContext context;
        f_->playOneFrame(KeySet(Key::DOWN), &context);
        EXPECT_EQ(FieldRealtime::SimulationState::STATE_VANISHING, f_->simulationState());
    }

    // Since this should be quick, the current state must be PREPARING_NEXT.
    {
        FrameContext context;
        f_->playOneFrame(KeySet(Key::DOWN), &context);
        EXPECT_EQ(FieldRealtime::SimulationState::STATE_PREPARING_NEXT, f_->simulationState());
    }
}

TEST_F(FieldRealtimeTest, playOneFrameWithMultipleKey1)
{
    f_->skipLevelSelect();
    f_->skipPreparingNext();

    ASSERT_EQ(KumipuyoPos(3, 12, 0), f_->kumipuyoPos());

    FrameContext context;
    EXPECT_TRUE(f_->playOneFrame(KeySet(Key::RIGHT, Key::RIGHT_TURN), &context));

    EXPECT_EQ(KumipuyoPos(4, 12, 1), f_->kumipuyoPos());
}

TEST_F(FieldRealtimeTest, playOneFrameWithMultipleKey2)
{
    f_->skipLevelSelect();
    f_->skipPreparingNext();

    ASSERT_EQ(KumipuyoPos(3, 12, 0), f_->kumipuyoPos());

    FrameContext context;
    EXPECT_TRUE(f_->playOneFrame(KeySet(Key::LEFT, Key::LEFT_TURN), &context));

    EXPECT_EQ(KumipuyoPos(2, 12, 3), f_->kumipuyoPos());
}

TEST_F(FieldRealtimeTest, playOneFrameWithMultipleKey3)
{
    f_->skipLevelSelect();
    f_->skipPreparingNext();

    ASSERT_EQ(KumipuyoPos(3, 12, 0), f_->kumipuyoPos());

    FrameContext context;
    EXPECT_TRUE(f_->playOneFrame(KeySet(Key::DOWN, Key::RIGHT_TURN), &context));

    // Since Y = 12, you might think KEY_DOWN is ignored. This is not true.
    // After 1 frame, puyo should drop 1 cell.
    EXPECT_EQ(KumipuyoPos(3, 12, 1), f_->kumipuyoPos());
    EXPECT_TRUE(f_->playOneFrame(KeySet(), &context));
    EXPECT_EQ(KumipuyoPos(3, 11, 1), f_->kumipuyoPos());
}

TEST_F(FieldRealtimeTest, playOneFrameWithMultipleArrowKey1)
{
    f_->skipLevelSelect();
    f_->skipPreparingNext();

    ASSERT_EQ(KumipuyoPos(3, 12, 0), f_->kumipuyoPos());

    FrameContext context;
    EXPECT_TRUE(f_->playOneFrame(KeySet(Key::DOWN, Key::RIGHT), &context));

    // KEY_DOWN should be ignored.
    EXPECT_EQ(KumipuyoPos(4, 12, 0), f_->kumipuyoPos());
}

TEST_F(FieldRealtimeTest, playOneFrameWithMultipleArrowKey2)
{
    f_->skipLevelSelect();
    f_->skipPreparingNext();

    ASSERT_EQ(KumipuyoPos(3, 12, 0), f_->kumipuyoPos());

    FrameContext context;
    EXPECT_TRUE(f_->playOneFrame(KeySet(Key::DOWN, Key::LEFT), &context));

    // KEY_DOWN should be ignored.
    EXPECT_EQ(KumipuyoPos(2, 12, 0), f_->kumipuyoPos());
}

TEST_F(FieldRealtimeTest, Move1)
{
    PlainField pf(
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

    f_->forceSetField(pf);
    f_->skipLevelSelect();
    f_->skipPreparingNext();

    f_->setKeySetSeq(PuyoController::findKeyStrokeFrom(CoreField(f_->field()), f_->kumipuyoMovingState(), Decision(5, 1)));
    while (true) {
        FrameContext context;
        KeySet keySet = f_->frontKeySet();
        f_->dropFrontKeySet();
        if (!f_->playOneFrame(keySet, &context))
            break;
    }

    PlainField expected(
        "    RR" // 12
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

    EXPECT_EQ(expected, f_->field());
}

TEST_F(FieldRealtimeTest, Move2)
{
    PlainField pf(
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

    f_->forceSetField(pf);
    f_->skipLevelSelect();
    f_->skipPreparingNext();

    f_->setKeySetSeq(PuyoController::findKeyStrokeFrom(CoreField(f_->field()), f_->kumipuyoMovingState(), Decision(6, 2)));
    while (true) {
        FrameContext context;
        KeySet keySet = f_->frontKeySet();
        f_->dropFrontKeySet();
        if (!f_->playOneFrame(keySet, &context))
            break;
    }

    PlainField expected(
        "     R"
        "     R" // 12
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

    EXPECT_EQ(expected, f_->field());
}

TEST_F(FieldRealtimeTest, Move3)
{
    PlainField pf(
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

    f_->forceSetField(pf);
    f_->skipLevelSelect();
    f_->skipPreparingNext();

    f_->setKeySetSeq(PuyoController::findKeyStrokeFrom(CoreField(f_->field()), f_->kumipuyoMovingState(), Decision(1, 2)));
    while (true) {
        FrameContext context;
        KeySet keySet = f_->frontKeySet();
        f_->dropFrontKeySet();
        if (!f_->playOneFrame(keySet, &context))
            break;
    }

    PlainField expected(
        "R     "
        "R     " // 12
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

    EXPECT_EQ(expected, f_->field());
}

TEST_F(FieldRealtimeTest, Move4)
{
    PlainField pf(
        "    OO" // 12
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

    f_->forceSetField(pf);
    f_->skipLevelSelect();
    f_->skipPreparingNext();

    f_->setKeySetSeq(PuyoController::findKeyStrokeFrom(CoreField(f_->field()), f_->kumipuyoMovingState(), Decision(6, 3)));
    while (true) {
        FrameContext context;
        KeySet keySet = f_->frontKeySet();
        f_->dropFrontKeySet();
        if (!f_->playOneFrame(keySet, &context))
            break;
    }

    PlainField expected(
        "    RR" // 13
        "    OO" // 12
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

    EXPECT_EQ(expected, f_->field());
}
