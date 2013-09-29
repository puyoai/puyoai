#ifndef CORE_PUYO_H_
#define CORE_PUYO_H_

#include <glog/logging.h>

enum PuyoColor {
  EMPTY = 0,
  OJAMA = 1,
  WALL = 2,
  RED = 4,
  BLUE = 5,
  YELLOW = 6,
  GREEN = 7,
};

const int PUYO_COLORS = 8;
const int PUYO_ERASE_NUM = 4;

typedef unsigned char Puyo;

// For backward compatibility
typedef PuyoColor Colors;


const int NUM_NORMAL_PUYO_COLORS = 4;

inline PuyoColor normalPuyoColorOf(int index)
{
    DCHECK(0 <= index && index < NUM_NORMAL_PUYO_COLORS);
    return static_cast<PuyoColor>(index + 4);
}

// Returns puyo color from character.
PuyoColor puyoColorOf(char c);

// Converts PuyoColor to character.
char charOfPuyoColor(PuyoColor c);

// Returns true if puyo is normal color puyo.
inline bool isColorPuyo(PuyoColor color)
{
    return RED <= color && color <= GREEN;
}

#endif
