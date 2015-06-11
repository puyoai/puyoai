#pragma once

#include <vector>

#include "core/player_state.h"
#include "cpu/peria/possible_rensa.h"

namespace peria {

struct PlayerHands {
  // Rensa which needs key puyos to fill.
  std::vector<PossibleRensa> need_keys;

  // Rensa which can be fired in known puyos.
  std::vector<PossibleRensa> firables;
};

}  // namespace peria
