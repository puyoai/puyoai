#pragma once

namespace peria {

struct PossibleRensa {
  inline int eval() const {
    return score - num_required_puyos * 1000;
  }

  int frames = 0;
  int score = 0;

  // The number of required but invisible puyos.
  int num_required_puyos = 0;
};

}  // namespace peria
