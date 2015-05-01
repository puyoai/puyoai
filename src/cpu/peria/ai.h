#pragma once

#include "core/client/ai/ai.h"

#include <memory>  // available with C++11

class RefPlan;

namespace peria {

class Ai : public ::AI {
 public:
  Ai(int argc, char* argv[]);
  virtual ~Ai();

 protected:
  struct Attack;
  struct Control;

  virtual DropDecision think(int frame_id,
                             const CoreField& field,
                             const KumipuyoSeq& seq,
                             const PlayerState& me,
                             const PlayerState& enemy,
                             bool fast) const override;
  virtual void onGameWillBegin(const FrameRequest& frame_request) override;
  virtual void onEnemyGrounded(const FrameRequest& frame_request) override;

  // Callback function to evaluate a controled state.
  static void EvaluatePlan(const RefPlan& plan,
                           Attack* attack,
                           const RensaChainTrackResult& track,
                           Control* control);

  static int PatternMatch(const RefPlan& plan, std::string* name);

  // Information about opponent's attacks.
  std::unique_ptr<Attack> attack_;
};

}  // namespace peria
