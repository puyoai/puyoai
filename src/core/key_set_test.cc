#include "core/key_set.h"

#include <gtest/gtest.h>

using namespace std;

TEST(KeySetTest, toInt)
{
    // When you change this value, you need to change arduino/puyo_controller/puyo_controller.ino
    EXPECT_EQ(1 << 0, KeySet(Key::UP).toInt());
    EXPECT_EQ(1 << 1, KeySet(Key::RIGHT).toInt());
    EXPECT_EQ(1 << 2, KeySet(Key::DOWN).toInt());
    EXPECT_EQ(1 << 3, KeySet(Key::LEFT).toInt());
    EXPECT_EQ(1 << 4, KeySet(Key::RIGHT_TURN).toInt());
    EXPECT_EQ(1 << 5, KeySet(Key::LEFT_TURN).toInt());
    EXPECT_EQ(1 << 6, KeySet(Key::START).toInt());
}
