#include "core/key.h"

using namespace std;

string toString(Key key)
{
    switch (key) {
    case KEY_NONE:        return "<NONE>";
    case KEY_UP:          return "↑";
    case KEY_RIGHT:       return "→";
    case KEY_DOWN:        return "↓";
    case KEY_LEFT:        return "←";
    case KEY_RIGHT_TURN:  return "A";
    case KEY_LEFT_TURN:   return "B";
    case KEY_START:       return "Start";
    }

    return "";
}
