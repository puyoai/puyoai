#include "cpu/peria/ai.h"

namespace peria {

Ai::Ai(): ::AI("peria") {}

Ai::~Ai() {}

DropDecision Ai::think(int /*frame_id*/,
		       const PlainField& /*field*/,
		       const Kumipuyo& /*next1*/,
		       const Kumipuyo& /*next2*/) {
  return DropDecision();
}

}  // namespace peria
