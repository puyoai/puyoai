#pragma once

#include "core/client/ai/ai.h"

class CoreField;
class RefPlan;

namespace munetoshi {

class AI : public ::AI {
 public:
  AI(int argc, char* argv[]);
  virtual ~AI() = default;

  DropDecision think(int frame_id, const CoreField& field,
                     const KumipuyoSeq& seq,
                     const AdditionalThoughtInfo&,
                     bool fast) override;

  enum GradeElement {
	  CHAIN_LENGTH,
	  NUM_REQUIRED_PUYO,
	  DEATH_RATIO,
	  TEAR,
	  GRACE_VALLEY_2_1,
	  GRACE_VALLEY_3_2,
	  GRACE_VALLEY_3_4,
	  GRACE_VALLEY_4_5,
	  GRACE_VALLEY_5_6,
	  GRACE_VALLEY_4_3_GT2,
	  GRADE_NUM,
  };

 protected:
  enum Strategy {
    FIRE,
    GROW,
  };

  virtual void onGameWillBegin(const FrameRequest&) override;

  virtual DropDecision think_internal(int frame_id, const CoreField& field,
                                      const KumipuyoSeq& seq);

  virtual void onEnemyGrounded(const FrameRequest&) override;

  virtual int evaluate(const CoreField& field, const RefPlan* plan);

  Strategy strategy;
};

}  // namespace munetoshi
