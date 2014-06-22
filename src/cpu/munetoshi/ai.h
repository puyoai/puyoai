#pragma once

#include "core/client/ai/ai.h"

namespace munetoshi {

class AI : public ::AI {
 public:
  AI();
  virtual ~AI() = default;

  DropDecision think(int frame_id, const PlainField& field,
                     const Kumipuyo& next1, const Kumipuyo& next2) override;

 protected:
  enum Strategy {
    FIRE,
    GROW,
  };

  virtual void gameWillBegin(const FrameData&) override;

  virtual DropDecision think_internal(int frame_id, const PlainField& field,
                                      const Kumipuyo& next1,
                                      const Kumipuyo& next2);

  virtual void enemyGrounded(const FrameData&) override;

  virtual int evaluate(const PlainField& field);

  Strategy strategy;
};

}  // namespace munetoshi
