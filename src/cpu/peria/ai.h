#pragma once

#include "core/client/ai/ai.h"

#include <memory>  // available with C++11

namespace peria {

class Ai : public ::AI {
 public:
  Ai(int argc, char* argv[]);
  virtual ~Ai();

 protected:
  struct Attack;

  virtual DropDecision think(int frame_id,
                             const CoreField& field,
                             const KumipuyoSeq& seq,
                             const AdditionalThoughtInfo& info,
                             bool fast) override;
  virtual void onGameWillBegin(const FrameRequest& frame_request) override;
  virtual void onEnemyGrounded(const FrameRequest& frame_request) override;

  // Information about opponent's attacks.
  std::unique_ptr<Attack> attack_;
};

}  // namespace peria
