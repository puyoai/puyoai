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
                             const AdditionalThoughtInfo& info,
                             bool fast) override;
  virtual void onGameWillBegin(const FrameRequest& frame_request) override;
  virtual void onEnemyGrounded(const FrameRequest& frame_request) override;

  static int PatternMatch(const RefPlan& plan, std::string* name);
  static void Evaluate(const RefPlan& plan, Attack* attack, Control* control);

  // Information about opponent's attacks.
  std::unique_ptr<Attack> attack_;
};

}  // namespace peria
