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

TEST_F(FieldRealtimeTest, state)
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

    // Then, 10 frames ground animation.
    for (int i = 0; i < 10; ++i) {
        FrameContext context;
        f_->playOneFrame(KeySet(Key::KEY_DOWN), &context);
        if (i == 0) {
            // state might be STATE_DROPPING just after STATE_PLAYABLE. It is OK.
            EXPECT_TRUE(f_->simulationState() == FieldRealtime::SimulationState::STATE_GROUNDING ||
                        f_->simulationState() == FieldRealtime::SimulationState::STATE_DROPPING);
        } else {
            EXPECT_EQ(FieldRealtime::SimulationState::STATE_GROUNDING, f_->simulationState());
        }
    }

    // Then, preparing next.
    FrameContext context;
    f_->playOneFrame(KeySet(Key::KEY_DOWN), &context);
    EXPECT_EQ(FieldRealtime::SimulationState::STATE_PREPARING_NEXT, f_->simulationState());

    // Here, score must be 13. 12 frames dropping + 1 frame grounding.
    EXPECT_EQ(13, f_->score());
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
        Key key = f_->getKey(Decision(5, 1));
        if (!f_->playOneFrame(KeySet(key), &context))
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
        Key key = f_->getKey(Decision(6, 2));
        if (!f_->playOneFrame(KeySet(key), &context))
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
        Key key = f_->getKey(Decision(1, 2));
        if (!f_->playOneFrame(KeySet(key), &context))
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
        Key key = f_->getKey(Decision(6, 3));
        if (!f_->playOneFrame(KeySet(key), &context))
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
