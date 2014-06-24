#ifndef CPU_PERIA_H_
#define CPU_PERIA_H_

#include "core/client/ai/ai.h"

namespace peria {

class Ai : public ::AI {
 public:
  Ai();
  virtual ~Ai();

  DropDecision think(int frame_id, const PlainField& field,
		     const Kumipuyo& next1, const Kumipuyo& next2) override;
};
 
}  // namespace peria

#endif // CPU_PERIA_H_
