#include "core/player_state.h"

#include <gtest/gtest.h>

TEST(PlayerStateTest, totalOjama1)
{
    PlayerState me;
    me.fixedOjama = 10;
    me.pendingOjama = 20;

    PlayerState enemy;

    EXPECT_EQ(10 + 20, me.totalOjama(enemy));
    EXPECT_EQ(0, enemy.totalOjama(me));
}

TEST(PlayerStateTest, totalOjama2)
{
    PlayerState me;
    me.fixedOjama = 10;
    me.pendingOjama = 20;

    PlayerState enemy;
    enemy.currentRensaResult = RensaResult(5, 70 * 90, 100, false);

    EXPECT_EQ(10 + 20 + 90, me.totalOjama(enemy));
    EXPECT_EQ(0, enemy.totalOjama(me));
}

TEST(PlayerStateTest, totalOjama3)
{
    PlayerState me;
    me.fixedOjama = 10;
    me.pendingOjama = 20;
    me.currentRensaResult = RensaResult(5, 70 * 90, 100, false);

    PlayerState enemy;

    EXPECT_EQ(0, me.totalOjama(enemy));
    EXPECT_EQ(90 - 30, enemy.totalOjama(me));
}
