#ifndef CORE_PUYO_COLOR_H_
#define CORE_PUYO_COLOR_H_

#include <string>
#include <stdint.h>
#include <glog/logging.h>
#include "base/base.h"

enum class PuyoColor : uint8_t {
    EMPTY = 0,
    OJAMA = 1,
    WALL = 2,
    IRON = 3,
    RED = 4,
    BLUE = 5,
    YELLOW = 6,
    GREEN = 7,
};

const int NUM_PUYO_COLORS = 8;
const int PUYO_ERASE_NUM = 4;
const int NUM_NORMAL_PUYO_COLORS = 4;

constexpr PuyoColor NORMAL_PUYO_COLORS[] {
    PuyoColor::RED, PuyoColor::BLUE, PuyoColor::YELLOW, PuyoColor::GREEN
};

// Returns puyo color from character.
PuyoColor toPuyoColor(char);

constexpr int ordinal(PuyoColor c) { return static_cast<int>(c); }
// Converts PuyoColor to character.
char toChar(PuyoColor, char charIfEmpty = ' ');
std::string toString(PuyoColor);

// Returns true if puyo is normal color puyo.
constexpr bool isNormalColor(PuyoColor color)
{
    return ordinal(PuyoColor::RED) <= ordinal(color) && ordinal(color) <= ordinal(PuyoColor::GREEN);
}

inline std::ostream& operator<<(std::ostream& os, PuyoColor c)
{
    return os << toChar(c);
}

#endif
