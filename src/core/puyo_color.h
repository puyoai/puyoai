#ifndef CORE_PUYO_COLOR_H_
#define CORE_PUYO_COLOR_H_

#include <string>
#include <stdint.h>
#include <glog/logging.h>
#include "base/base.h"

// TODO(mayah): Consider use enum class.
enum PuyoColor : uint8_t {
    EMPTY = 0,
    OJAMA = 1,
    WALL = 2,
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

// Converts PuyoColor to character.
char toChar(PuyoColor, char charIfEmpty = ' ');

std::string toString(PuyoColor);

// Returns true if puyo is normal color puyo.
constexpr bool isNormalColor(PuyoColor color)
{
    return static_cast<int>(PuyoColor::RED) <= static_cast<int>(color) && static_cast<int>(color) <= static_cast<int>(PuyoColor::GREEN);
}

#endif
