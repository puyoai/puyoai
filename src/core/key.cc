#include "core/key.h"

#include <glog/logging.h>

#include <ostream>

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

Key toKey(char c)
{
    switch (c) {
    case '^': return Key::UP;
    case '>': return Key::RIGHT;
    case 'v': return Key::DOWN;
    case '<': return Key::LEFT;
    case 'A': return Key::RIGHT_TURN;
    case 'B': return Key::LEFT_TURN;
    case 'S': return Key::START;
    default:
        CHECK(false) << "Unknown key: " << c;
    }
}
