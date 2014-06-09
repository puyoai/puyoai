#ifndef CORE_KEY_H_
#define CORE_KEY_H_

#include <string>

enum Key {
    KEY_NONE,
    KEY_UP,
    KEY_RIGHT,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT_TURN,
    KEY_LEFT_TURN,
    KEY_START
};

std::string toString(Key);

#endif
