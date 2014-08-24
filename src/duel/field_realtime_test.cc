#include "duel/field_realtime.h"

#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "core/constant.h"
#include "core/kumipuyo.h"
#include "core/state.h"
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
    for (int i = 0; i < 6; ++i) {
        FrameContext context;
        f_->playOneFrame(KeySet(Key::KEY_DOWN), &context);
        EXPECT_EQ(FieldRealtime::SimulationState::STATE_PREPARING_NEXT, f_->simulationState());
    }

    // It needs 12 frames to reach the bottom.
    for (int i = 0; i < 12; ++i) {
        FrameContext context;
        f_->playOneFrame(KeySet(Key::KEY_DOWN), &context);
        EXPECT_EQ(FieldRealtime::SimulationState::STATE_PLAYABLE, f_->simulationState());
    }

    // Then, 1 frame dropping. (Even without dropping, AC puyo 2 has 1 frame here.
    // Actually it might have to be counted as PLAYABLE, though.
    {
        FrameContext context;
        f_->playOneFrame(KeySet(Key::KEY_DOWN), &context);
        EXPECT_EQ(FieldRealtime::SimulationState::STATE_DROPPING, f_->simulationState());
    }

    // Then, 10 frames ground animation.
    for (int i = 0; i < 10; ++i) {
        FrameContext context;
        f_->playOneFrame(KeySet(Key::KEY_DOWN), &context);
        EXPECT_EQ(FieldRealtime::SimulationState::STATE_GROUNDING, f_->simulationState());
    }

    // Then, preparing next.
    FrameContext context;
    f_->playOneFrame(KeySet(Key::KEY_DOWN), &context);
    EXPECT_EQ(FieldRealtime::SimulationState::STATE_PREPARING_NEXT, f_->simulationState());

    // Here, score must be 13. 12 frames dropping + 1 frame grounding.
    EXPECT_EQ(13, f_->score());
}

TEST_F(FieldRealtimeTest, zenkeshi)
{
    f_->forceSetField(CoreField("  RR  "));
    f_->skipLevelSelect();

    // Waiting next (6) + reaching bottom (11) + dropping (1) + grounding (10)
    for (int i = 0; i < 6 + 11 + 1 + 10; ++i) {
        FrameContext context;
        f_->playOneFrame(KeySet(Key::KEY_DOWN), &context);
        cout << (int)f_->simulationState() << endl;
        EXPECT_FALSE(f_->hasZenkeshi());
    }

    FrameContext context;
    f_->playOneFrame(KeySet(Key::KEY_DOWN), &context);
    EXPECT_TRUE(f_->hasZenkeshi());
    EXPECT_EQ(0, context.numSentOjama());

    for (int i = 0; i < 24; ++i) {
        FrameContext context;
        f_->playOneFrame(KeySet(Key::KEY_DOWN), &context);
        EXPECT_EQ(FieldRealtime::SimulationState::STATE_VANISHING, f_->simulationState());
    }

    // Since this should be quick, the current state must be PREPARING_NEXT.
    {
        FrameContext context;
        f_->playOneFrame(KeySet(Key::KEY_DOWN), &context);
        EXPECT_EQ(FieldRealtime::SimulationState::STATE_PREPARING_NEXT, f_->simulationState());
    }
}

TEST_F(FieldRealtimeTest, Move1)
{
    CoreField cf(
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

    f_->forceSetField(cf);
    f_->skipLevelSelect();
    f_->skipPreparingNext();

    while (true) {
        FrameContext context;
        KeySet keySet = f_->getKeySet(Decision(5, 1));
        if (!f_->playOneFrame(keySet, &context))
            break;
    }

    CoreField expected(
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
    CoreField cf(
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

    f_->forceSetField(cf);
    f_->skipLevelSelect();
    f_->skipPreparingNext();

    while (true) {
        FrameContext context;
        KeySet keySet = f_->getKeySet(Decision(6, 2));
        if (!f_->playOneFrame(keySet, &context))
            break;
    }

    CoreField expected(
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
    CoreField cf(
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

    f_->forceSetField(cf);
    f_->skipLevelSelect();
    f_->skipPreparingNext();

    while (true) {
        FrameContext context;
        KeySet keySet = f_->getKeySet(Decision(1, 2));
        if (!f_->playOneFrame(keySet, &context))
            break;
    }

    CoreField expected(
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
    CoreField cf(
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

    f_->forceSetField(cf);
    f_->skipLevelSelect();
    f_->skipPreparingNext();

    while (true) {
        FrameContext context;
        KeySet keySet = f_->getKeySet(Decision(6, 3));
        if (!f_->playOneFrame(keySet, &context))
            break;
    }

    CoreField expected(
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
