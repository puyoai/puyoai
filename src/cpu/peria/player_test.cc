#include "player.h"

#include <glog/logging.h>
#include <gtest/gtest.h>
#include <string>
#include <vector>

#include "base.h"

class TestablePlayer : public Player {
 public:
  using Player::GetControls;
};

class PlayerTest : public testing::Test {
};

TEST_F(PlayerTest, GetControls) {
  struct TestData {
    const char* field;  // input
    size_t ex_number;   // expected number of controls
  } datas[] = {
    {"", 22},
    {"100000" "100000" "100000" "100000" "100000" "100000"
     "100000" "100000" "100000" "100000" "100000" "100000", 18},
    {"100001" "100001" "100001" "100001" "100001" "100001"
     "100001" "100001" "100001" "100001" "100001" "100001", 14},
    {"100000" "100001" "100001" "100001" "100001" "100001"
     "100001" "100001" "100001" "100001" "100001" "100001", 21},
  };

  for (int i = 0; i < ARRAYSIZE(datas); ++i) {
    vector<Player::Control> controls;
    TestablePlayer player;
    player.set_field(Field(datas[i].field));
    player.GetControls(&controls);
    EXPECT_EQ(datas[i].ex_number, controls.size());
  }
}

TEST_F(PlayerTest, ApplyControl) {
  struct TestData {
    const char* field;     // Input
    const char* sequence;  // Input
    int x;                 // Input
    int r;                 // Input
    double ex_score;       // Expected least score (depends on scoring function)
  } datas[] = {
    {"RR", "RR", 1, 0, 0},
    {"RR", "RR", 6, 0, 40},
  };

  for (int i = 0; i < ARRAYSIZE(datas); ++i) {
    Player player;
    player.set_field(Field(datas[i].field));
    player.SetColorSequence(datas[i].sequence);
    Player::Control control(datas[i].x, datas[i].r);
    EXPECT_LE(datas[i].ex_score, player.ApplyControl(control, NULL));
  }
}
