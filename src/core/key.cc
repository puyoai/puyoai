#include "core/key.h"

#include <glog/logging.h>

using namespace std;

string toString(Key key)
{
    switch (key) {
    case Key::UP:          return "\xE2\x86\x91";  // "↑"
    case Key::RIGHT:       return "\xE2\x86\x92";  // "→"
    case Key::DOWN:        return "\xE2\x86\x93";  // "↓"
    case Key::LEFT:        return "\xE2\x86\x90";  // "←"
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
