#include "player.h"

#include <gtest/gtest.h>

#include "base.h"

class PlayerTest : public testing::Test {
};

TEST_F(PlayerTest, Get) {
  Player player;
  player.set_field(Field("444400"));
  EXPECT_EQ(kRed, player.field().Get(1, 1));
}
