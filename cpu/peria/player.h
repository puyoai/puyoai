#ifndef CPU_PERIA_PLAYER_H_
#define CPU_PERIA_PLAYER_H_

#include "base.h"
#include "field.h"

struct Player {
 public:
  enum State {
    kNone = 0,
    kPlay = 1 << 0,
    kNext2 = 1 << 2,
    kSet = 1 << 4,
    kWin = 1 << 6,
    kChainEnd = 1 << 8,
    kAll = 0x55555555u,
  };

  void CopyFrom(const Player& player);
  
  int state;
  Field field;
  int score;
  int x;  // x-axis of Jiku puyo
  int y;  // y-axis of Jiku puyo
  int r;  // round number of controlling puyo.
  int ojama;
};

bool operator==(const Player& a, const Player& b);
bool operator!=(const Player& a, const Player& b);

#endif  // CPU_PERIA_PLAYER_H_
