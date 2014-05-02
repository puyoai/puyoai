#include "field_realtime.h"

#include <string>

#include <gtest/gtest.h>

#include <core/constant.h>
#include <core/state.h>
#include "field_realtime.h"
#include "ojama_controller.h"

using namespace std;

class FieldRealtimeTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    string sequence = "11223344";
    ojama_controller_ = new OjamaController();
    f_ = new FieldRealtime(0, sequence, ojama_controller_);
  }

  virtual void TearDown() {
    delete ojama_controller_;
    delete f_;
  }

  FieldRealtime* f_;
  PlayerLog player_log_;
  OjamaController* ojama_controller_;
};

TEST_F(FieldRealtimeTest, TimingAfterKeyInput) {
  Key key = KEY_RIGHT;

  EXPECT_EQ(FieldRealtime::STATE_USER, f_->GetSimulationState());
  f_->Play(key, &player_log_);
  EXPECT_EQ(FieldRealtime::STATE_SLEEP, f_->GetSimulationState());
  f_->Play(key, &player_log_);
  EXPECT_EQ(FieldRealtime::STATE_USER, f_->GetSimulationState());
}

TEST_F(FieldRealtimeTest, DISABLED_TimingFreeFall) {
  vector<int> states;
  // Free fall.
  for (int i = 0; i < 12; i++) {
    for (int i = 0; i < FRAMES_FREE_FALL; i++) {
      states.push_back(FieldRealtime::STATE_USER);
    }
  }
  // Wait after ground.
  for (int i = 0; i < FRAMES_AFTER_GROUND; i++) {
    states.push_back(FieldRealtime::STATE_SLEEP);
  }
  // Chigiri.
  states.push_back(FieldRealtime::STATE_CHIGIRI);
  for (int i = 0; i < FRAMES_AFTER_NO_CHIGIRI; i++) {
    states.push_back(FieldRealtime::STATE_SLEEP);
  }
  states.push_back(FieldRealtime::STATE_VANISH);
  for (int i = 0; i < FRAMES_AFTER_VANISH; i++) {
    states.push_back(FieldRealtime::STATE_SLEEP);
  }
  // Started free fall of next puyo.
  states.push_back(FieldRealtime::STATE_USER);

  for (int i = 0; i < states.size(); i++) {
    EXPECT_EQ(states[i], f_->GetSimulationState()) << "index=" << i;
    f_->Play(KEY_NONE, &player_log_);
  }
}

TEST_F(FieldRealtimeTest, DISABLED_TimingChigiri) {
  for (int i = 0 ; i < 11; i++) {
    f_->Play(KEY_DOWN, &player_log_);
    f_->Play(KEY_NONE, &player_log_);
  }
  // Now the puyo is at [3,1] and [3,2], but not ground yet.
  // Ground the puyo.
  EXPECT_EQ(FieldRealtime::STATE_USER, f_->GetSimulationState());
  f_->Play(KEY_DOWN, &player_log_);

  // Wait for a few frames before chigiri starts.
  for (int i = 0; i < FRAMES_AFTER_GROUND; i++) {
    EXPECT_EQ(FieldRealtime::STATE_SLEEP, f_->GetSimulationState());
    f_->Play(KEY_NONE, &player_log_);
  }

  // Chigiri started.
  EXPECT_EQ(FieldRealtime::STATE_CHIGIRI, f_->GetSimulationState());
  f_->Play(KEY_NONE, &player_log_);
  EXPECT_EQ(FieldRealtime::STATE_SLEEP, f_->GetSimulationState());
  f_->Play(KEY_NONE, &player_log_);
  EXPECT_EQ(FieldRealtime::STATE_VANISH, f_->GetSimulationState());
  for (int i = 0; i < FRAMES_AFTER_VANISH; i++) {
    f_->Play(KEY_NONE, &player_log_);
    EXPECT_EQ(FieldRealtime::STATE_SLEEP, f_->GetSimulationState());
  }
  f_->Play(KEY_NONE, &player_log_);
  EXPECT_EQ(FieldRealtime::STATE_USER, f_->GetSimulationState());
}
