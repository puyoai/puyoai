#include "core/key.h"

using namespace std;

string toString(Key key)
{
    switch (key) {
    case Key::UP:          return "↑";
    case Key::RIGHT:       return "→";
    case Key::DOWN:        return "↓";
    case Key::LEFT:        return "←";
    case Key::RIGHT_TURN:  return "A";
    case Key::LEFT_TURN:   return "B";
    case Key::START:       return "Start";
    }

    return "";
}
