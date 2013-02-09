#include "player.h"

#include <gtest/gtest.h>

#include "base.h"

class PlayerTest : public testing::Test {
 protected:
  virtual void SetUp() {
    player_.reset(new Player());
  }
  virtual void TearDown() {
    player_.reset();
  }

 public:
  scoped_ptr<Player> player_;
};

TEST_F(PlayerTest, Get) {
  player_->set_field(Field("4444"));
  EXPECT_EQ(kRed, player_->field().Get(1, 1));
}
