#ifndef CORE_KEY_H_
#define CORE_KEY_H_

#include <string>

enum class Key {
    KEY_UP,
    KEY_RIGHT,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT_TURN,
    KEY_LEFT_TURN,
    KEY_START,
};
const int NUM_KEYS = 7;

std::string toString(Key);
inline int ordinal(Key key) { return static_cast<int>(key); }

#endif
