#pragma once

namespace peria {

struct PossibleRensa {
  inline int eval() const {
    return score - num_required_puyos * 1000;
  }

  int frames;
  int score;

  // The number of required but invisible puyos.
  int num_required_puyos;
};

}  // namespace peria
