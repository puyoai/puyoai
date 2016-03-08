#pragma once

#include "core/client/ai/ai.h"

#include <memory>

namespace peria {

// Pai = peria's AI
class Pai : public ::AI {
 public:
  Pai(int argc, char* argv[]);
  virtual ~Pai();

 protected:
  virtual DropDecision think(int frame_id,
                             const CoreField& field,
                             const KumipuyoSeq& seq,
                             const PlayerState& me,
                             const PlayerState& enemy,
                             bool fast) const override;
};

}  // namespace peria
