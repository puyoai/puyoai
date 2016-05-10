#pragma once

#include "core/client/ai/ai.h"

namespace peria {

class BasicAi : public ::AI {
 public:
  BasicAi(int argc, char* argv[]);
  virtual ~BasicAi();

 protected:
  virtual DropDecision think(int frame_id,
                             const CoreField& field,
                             const KumipuyoSeq& seq,
                             const PlayerState& me,
                             const PlayerState& enemy,
                             bool fast) const override;
};

}  // namespace peria
