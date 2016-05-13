#pragma once

#include "core/client/ai/ai.h"

class Decisoin;
class KumipuyoSeq;
class RefPlan;
struct PlayerState;

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

 private:
  static void UsualSearch(const KumipuyoSeq& seq, const PlayerState& myState, const PlayerState& enemyState, int frameId,
                          const RefPlan& plan, double* bestScore, Decision* bestDecision);
};

}  // namespace peria
