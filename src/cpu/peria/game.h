#ifndef PERIA_GAME_H_
#define PERIA_GAME_H_

#include "cpu/peria/player.h"

namespace peria {

// Describes a game status.
struct Game {
  int id;
  Player players[2];  // [0]: Player, [1]: Enemy
};

}  // namespace peria

#endif  // PERIA_GAME_H_
