#include "core/key_set.h"

#include <gtest/gtest.h>

using namespace std;

TEST(KeySetTest, toInt)
{
    // When you change this value, you need to change arduino/puyo_controller/puyo_controller.ino
    EXPECT_EQ(1 << 0, KeySet(Key::KEY_UP).toInt());
    EXPECT_EQ(1 << 1, KeySet(Key::KEY_RIGHT).toInt());
    EXPECT_EQ(1 << 2, KeySet(Key::KEY_DOWN).toInt());
    EXPECT_EQ(1 << 3, KeySet(Key::KEY_LEFT).toInt());
    EXPECT_EQ(1 << 4, KeySet(Key::KEY_RIGHT_TURN).toInt());
    EXPECT_EQ(1 << 5, KeySet(Key::KEY_LEFT_TURN).toInt());
    EXPECT_EQ(1 << 6, KeySet(Key::KEY_START).toInt());
}

TEST(KeySetTest, basic)
{
    vector<KeySet> keySets {
        KeySet(Key::KEY_RIGHT),
        KeySet(Key::KEY_LEFT),
        KeySet(Key::KEY_DOWN),
        KeySet(Key::KEY_RIGHT_TURN),
        KeySet(Key::KEY_LEFT_TURN),
    };

    EXPECT_EQ(">,<,v,A,B", KeySetSeq(keySets).toString());
}

TEST(KeySetTest, tuple)
{
    vector<KeySet> keySets {
        KeySet(Key::KEY_RIGHT, Key::KEY_RIGHT_TURN),
        KeySet(Key::KEY_RIGHT_TURN, Key::KEY_RIGHT),
        KeySet(Key::KEY_LEFT, Key::KEY_LEFT_TURN),
        KeySet(Key::KEY_LEFT_TURN, Key::KEY_LEFT),
        KeySet(Key::KEY_DOWN, Key::KEY_RIGHT_TURN),
        KeySet(Key::KEY_DOWN, Key::KEY_LEFT_TURN),
    };

    EXPECT_EQ(">A,>A,<B,<B,vA,vB", KeySetSeq(keySets).toString());
}
