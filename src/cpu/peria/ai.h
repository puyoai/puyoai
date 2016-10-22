#pragma once

#include "core/client/ai/ai.h"
#include "cpu/peria/player_hands.h"

#include <memory>  // available with C++11

class RefPlan;

namespace peria {

class Evaluator;

struct Attack {
  int ojama;
  int end_frame;
};

class Ai : public ::AI {
 public:
  Ai(int argc, char* argv[]);
  virtual ~Ai();

 protected:
  virtual DropDecision think(int frame_id,
                             const CoreField& field,
                             const KumipuyoSeq& seq,
                             const PlayerState& me,
                             const PlayerState& enemy,
                             bool fast) const override;
  virtual void onGroundedForEnemy(const FrameRequest& frame_request) override;

  DropDecision checkJoseki(const CoreField& field, const KumipuyoSeq& seq) const;

  static void IterationCallback(int step, int start_frame, PlayerState me, PlayerState enemy, const KumipuyoSeq& next, Evaluator& evaluator, const RefPlan& plan);

  PlayerHands enemy_hands_;
};

}  // namespace peria
