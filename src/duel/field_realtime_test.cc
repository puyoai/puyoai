#include "duel/field_realtime.h"

#include <string>

#include <gtest/gtest.h>

#include "core/constant.h"
#include "core/kumipuyo.h"
#include "core/state.h"
#include "duel/field_realtime.h"

using namespace std;

class FieldRealtimeTest : public ::testing::Test {
protected:
    virtual void SetUp()
    {
        string sequence = "11223344";
        f_ = new FieldRealtime(0, sequence);
    }

    virtual void TearDown()
    {
        delete f_;
    }

    FieldRealtime* f_;
};

TEST_F(FieldRealtimeTest, TimingAfterKeyInput)
{
    Key key = KEY_RIGHT;

    EXPECT_EQ(FieldRealtime::STATE_USER, f_->simulationState());
    EXPECT_FALSE(f_->isSleeping());

    f_->playOneFrame(key, nullptr);
    EXPECT_EQ(FieldRealtime::STATE_USER, f_->simulationState());
    EXPECT_TRUE(f_->isSleeping());

    f_->playOneFrame(key, nullptr);
    EXPECT_EQ(FieldRealtime::STATE_USER, f_->simulationState());
    EXPECT_FALSE(f_->isSleeping());
}

TEST_F(FieldRealtimeTest, DISABLED_TimingFreeFall)
{
    vector<pair<int, bool>> states;

    // Free fall.
    for (int i = 0; i < 12; i++) {
        for (int i = 0; i < FRAMES_FREE_FALL; i++) {
            states.push_back(make_pair(FieldRealtime::STATE_USER, false));
        }
    }
    // Wait after ground.
    for (int i = 0; i < FRAMES_AFTER_GROUND; i++) {
        states.push_back(make_pair(FieldRealtime::STATE_CHIGIRI, true));
    }
    // Chigiri.
    states.push_back(make_pair(FieldRealtime::STATE_CHIGIRI, false));
    for (int i = 0; i < FRAMES_AFTER_NO_CHIGIRI; i++) {
        states.push_back(make_pair(FieldRealtime::STATE_VANISH, true));
    }
    states.push_back(make_pair(FieldRealtime::STATE_VANISH, false));
    for (int i = 0; i < FRAMES_AFTER_VANISH; i++) {
        states.push_back(make_pair(FieldRealtime::STATE_USER, false));
    }
    // Started free fall of next puyo.
    states.push_back(make_pair(FieldRealtime::STATE_USER, false));

    for (size_t i = 0; i < states.size(); i++) {
        EXPECT_EQ(states[i].first, f_->simulationState()) << "index=" << i;
        EXPECT_EQ(states[i].second, f_->isSleeping()) << "index=" << i;
        f_->playOneFrame(KEY_NONE, nullptr);
    }
}

TEST_F(FieldRealtimeTest, DISABLED_TimingChigiri)
{
    for (int i = 0 ; i < 11; i++) {
        f_->playOneFrame(KEY_DOWN, nullptr);
        f_->playOneFrame(KEY_NONE, nullptr);
    }

    // Now the puyo is at [3,1] and [3,2], but not ground yet.
    // Ground the puyo.
    EXPECT_EQ(FieldRealtime::STATE_USER, f_->simulationState());
    EXPECT_FALSE(f_->isSleeping());
    f_->playOneFrame(KEY_DOWN, nullptr);

    // Wait for a few frames before chigiri starts.
    for (int i = 0; i < FRAMES_AFTER_GROUND; i++) {
        EXPECT_EQ(FieldRealtime::STATE_CHIGIRI, f_->simulationState());
        EXPECT_TRUE(f_->isSleeping());
        f_->playOneFrame(KEY_NONE, nullptr);
    }

    // Chigiri started.
    EXPECT_EQ(FieldRealtime::STATE_CHIGIRI, f_->simulationState());
    EXPECT_FALSE(f_->isSleeping());

    f_->playOneFrame(KEY_NONE, nullptr);
    EXPECT_EQ(FieldRealtime::STATE_VANISH, f_->simulationState());
    EXPECT_TRUE(f_->isSleeping());

    f_->playOneFrame(KEY_NONE, nullptr);
    EXPECT_EQ(FieldRealtime::STATE_DROP, f_->simulationState());
    for (int i = 0; i < FRAMES_AFTER_VANISH; i++) {
        f_->playOneFrame(KEY_NONE, nullptr);
        EXPECT_TRUE(f_->isSleeping());
    }
    f_->playOneFrame(KEY_NONE, nullptr);
    EXPECT_EQ(FieldRealtime::STATE_USER, f_->simulationState());
}
