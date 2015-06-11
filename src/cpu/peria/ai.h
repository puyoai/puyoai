#pragma once

#include "core/client/ai/ai.h"
#include "cpu/peria/player_hands.h"

#include <memory>  // available with C++11

class RefPlan;

namespace peria {

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

  static int PatternMatch(const RefPlan& plan, std::string* name);

  PlayerHands enemy_hands_;
};

}  // namespace peria
