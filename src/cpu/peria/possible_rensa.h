#pragma once

namespace peria {

struct PossibleRensa {
  int frames;
  int score;

  // The number of required but invisible puyos.
  int num_required_puyos;
};

}  // namespace peria
