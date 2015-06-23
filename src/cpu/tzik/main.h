
#pragma once

#include "core/client/ai/ai.h"

namespace tzik {

class AI final : public ::AI {
 public:
  AI(int argc, char** argv);
  virtual ~AI() = default;

  DropDecision think(int frame_id,
                     const CoreField& field,
                     const KumipuyoSeq& seq,
                     const PlayerState& me,
                     const PlayerState& enemy,
                     bool fast) const override;

 private:
  DropDecision think_sample(int frame_id,
                            const CoreField& field,
                            const KumipuyoSeq& seq) const;

  AI(const AI&) = delete;
  AI(AI&&) = delete;
  AI& operator=(const AI&) = delete;
  AI& operator=(AI&&) = delete;
};

}  // namespace tzik
