#include "core/key_set.h"

KeySet::KeySet(Key key)
{
    switch (key) {
    case Key::KEY_DOWN:
        downKey = true;
        break;
    case Key::KEY_LEFT:
        leftKey = true;
        break;
    case Key::KEY_RIGHT:
        rightKey = true;
        break;
    case Key::KEY_LEFT_TURN:
        leftTurnKey = true;
        break;
    case Key::KEY_RIGHT_TURN:
        rightTurnKey = true;
        break;
    default:
        break;
    }
}
