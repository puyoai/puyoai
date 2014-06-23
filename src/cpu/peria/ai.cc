#include "cpu/peria/ai.h"

#include "cpu/peria/game.h"

namespace peria {

Ai::Ai() : name_("peria") {}

Ai::~Ai() {}

void Ai::RunLoop() {
  Game game;
  while (connector_.Receive(&game)) {}
}

}  // namespace peria
