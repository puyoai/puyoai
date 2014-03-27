#include "player.h"

#include <gtest/gtest.h>
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
    char* field;       // input
    size_t ex_number;  // expected number of controls
  } datas[] = {
    {"", 22},
  };

  for (int i = 0; i < ARRAYSIZE(datas); ++i) {
    vector<Player::Control> controls;
    TestablePlayer player;
    player.set_field(Field(string(datas[i].field)));
    player.GetControls(&controls);
    EXPECT_EQ(datas[i].ex_number, controls.size());
  }
}
