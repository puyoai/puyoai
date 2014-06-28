#ifndef CORE_KEY_SET_H_
#define CORE_KEY_SET_H_

#include "core/key.h"

struct KeySet {
    KeySet() {}
    explicit KeySet(Key);

    bool downKey = false;
    bool leftKey = false;
    bool rightKey = false;
    bool rightTurnKey = false;
    bool leftTurnKey = false;
};

#endif
