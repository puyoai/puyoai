#include "core/key.h"

using namespace std;

string toString(Key key)
{
    switch (key) {
    case Key::KEY_UP:          return "↑";
    case Key::KEY_RIGHT:       return "→";
    case Key::KEY_DOWN:        return "↓";
    case Key::KEY_LEFT:        return "←";
    case Key::KEY_RIGHT_TURN:  return "A";
    case Key::KEY_LEFT_TURN:   return "B";
    case Key::KEY_START:       return "Start";
    }

    return "";
}
