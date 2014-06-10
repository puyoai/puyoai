
#pragma once

#include "core/client/ai/ai.h"

namespace tzik {

class AI final : public ::AI {
 public:
  AI();
  virtual ~AI() = default;

  DropDecision think(int frame_id,
                     const PlainField& field,
                     const Kumipuyo& next1,
                     const Kumipuyo& next2) override;

 private:
  DropDecision think_sample(int frame_id,
                              const PlainField& field,
                              const Kumipuyo& next1,
                              const Kumipuyo& next2);


  AI(const AI&) = delete;
  AI(AI&&) = delete;
  AI& operator=(const AI&) = delete;
  AI& operator=(AI&&) = delete;
};

}  // namespace tzik
