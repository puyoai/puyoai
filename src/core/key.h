#ifndef CORE_KEY_H_
#define CORE_KEY_H_

#include <string>

enum class Key {
    UP,
    RIGHT,
    DOWN,
    LEFT,
    RIGHT_TURN,
    LEFT_TURN,
    START,
};
const int NUM_KEYS = 7;

std::string toString(Key);
constexpr int ordinal(Key key) { return static_cast<int>(key); }
Key toKey(char c);

#endif
