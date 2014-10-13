#include "cpu/peria/ai.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <vector>

#include "core/algorithm/plan.h"
#include "core/constant.h"
#include "core/frame_request.h"

#include "cpu/peria/evaluate.h"

namespace peria {


struct Ai::Attack {
  int score;
  int end_frame_id;
};

// TODO: (want to implement)
// - Search decisions for all known |seq|
// --- Count the number of HAKKA-able KumiPuyos

Ai::Ai(int argc, char* argv[]): ::AI(argc, argv, "peria") {}

Ai::~Ai() {}

DropDecision Ai::think(int frame_id,
                       const PlainField& field,
                       const KumipuyoSeq& seq,
                       const AdditionalThoughtInfo& info) {
  UNUSED_VARIABLE(info);
  using namespace std::placeholders;

  // TODO: Merge all Plan::iterateAvailablePlans() to reduce computing cost.
  
  // Check templates first with visible puyos.
  {
    int score = 0;
    std::string name;
    int frames = 1e+8;
    Decision temp_decision;
    Plan::iterateAvailablePlans(CoreField(field), seq, 2,
                                std::bind(Evaluate::Patterns, _1,
                                          &score, &name,
                                          &frames, &temp_decision));
    if (score > 200 && !name.empty())
      return DropDecision(temp_decision, "Template: " + name);
  }

  // 3 個以上おじゃまが来てたらカウンターしてみる
  if (attack_ && attack_->score >= SCORE_FOR_OJAMA * 3) {
    // TODO: Adjust |kAcceptablePuyo|.
    const int kAcceptablePuyo = 3;
    int threshold = attack_->score - SCORE_FOR_OJAMA * kAcceptablePuyo;

    int score = 0;
    Decision decision;
    Plan::iterateAvailablePlans(
        CoreField(field), seq, 2,
        std::bind(Evaluate::Counter, _1,
                  threshold,
                  attack_->end_frame_id - frame_id,
                  &score, &decision));
    if (threshold < score)
      return DropDecision(decision, "Counter");
  }

  // Default search
  {
    int score = 0;
    Decision decision;
    Plan::iterateAvailablePlans(
        CoreField(field), seq, 2,
        std::bind(Evaluate::Usual, _1, &score, &decision));

    return DropDecision(decision, "Normal");
  }

  // DO NOT REACH HERE
}

void Ai::onGameWillBegin(const FrameRequest& /*frame_request*/) {
  attack_.reset();
}

void Ai::onEnemyGrounded(const FrameRequest& frame_request) {
  const PlainField& enemy = frame_request.enemyPlayerFrameRequest().field;
  CoreField field(enemy);
  field.forceDrop();
  RensaResult result = field.simulate();

  if (result.chains == 0) {
    // TODO: Check required puyos to start RENSA.
    attack_.reset();
    return;
  }

  attack_.reset(new Attack);
  attack_->score = result.score;
  attack_->end_frame_id = frame_request.frameId + result.frames;
}

}  // namespace peria
