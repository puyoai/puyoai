#include "cpu/peria/ai.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <climits>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "base/base.h"
#include "core/algorithm/plan.h"
#include "core/algorithm/rensa_detector.h"
#include "core/constant.h"
#include "core/frame_request.h"

#include "cpu/peria/evaluator.h"

namespace peria {

struct Ai::Attack {
  int score;
  int end_frame_id;
};

struct Ai::Control {
  std::string message;
  int score = 0;
  Decision decision;
};

Ai::Ai(int argc, char* argv[]): ::AI(argc, argv, "peria") {}

Ai::~Ai() {}

DropDecision Ai::think(int frame_id,
                       const CoreField& field,
                       const KumipuyoSeq& seq,
                       const PlayerState& me,
                       const PlayerState& enemy,
                       bool fast) const {
  UNUSED_VARIABLE(frame_id);
  UNUSED_VARIABLE(me);
  UNUSED_VARIABLE(enemy);
  UNUSED_VARIABLE(fast);
  using namespace std::placeholders;

  // Currently planning rensa.
  RensaChainTrackResult track_result;

  int max_score = 0;
  RensaDetector::iteratePossibleRensasIteratively(
      field, 1, RensaDetectorStrategy::defaultFloatStrategy(),
      [&max_score, &track_result](const CoreField&,
                                  const RensaResult& result,
                                  const ColumnPuyoList&,
                                  const RensaChainTrackResult& track) {
        if (result.score > max_score) {
          max_score = result.score;
          track_result = track;
        }
      });

  // Look for plans.
  Control control;
  control.score = -10000;
  auto evaluate = std::bind(Ai::EvaluatePlan, _1, attack_.get(),
                            track_result, &control);
  Plan::iterateAvailablePlans(field, seq, 2, evaluate);

  DLOG(INFO) << control.message;
  DLOG(INFO) << field.toDebugString();
  return DropDecision(control.decision, control.message);
}

void Ai::onGameWillBegin(const FrameRequest& /*frame_request*/) {
  attack_.reset();
}

void Ai::onGroundedForEnemy(const FrameRequest& frame_request) {
  const PlainField& enemy = frame_request.enemyPlayerFrameRequest().field;
  CoreField field(CoreField::fromPlainFieldWithDrop(enemy));
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

void Ai::EvaluatePlan(const RefPlan& plan,
                      Attack* attack,
                      const RensaChainTrackResult& track,
                      Control* control) {
  // Score in total.
  int score = 0;
  std::ostringstream oss;

  // Score and a message for a feature.
  int value = 0;
  std::string message;

  // Near future expectation
  value = Evaluator::Future(plan.field());
  if (value) {
    oss << "Future(" << value << ")_";
    score += value;
  }

  // Pattern maching
  value = Evaluator::PatternMatch(plan.field(), &message);
  if (value) {
    oss << "Pattern(" << message << "=" << value << ")_";
    score += value;
  }

  // Field statement
  value = Evaluator::Field(plan.field());
  oss << "Field(" << value << ")";
  score += value;

  if (plan.isRensaPlan()) {
    const int kAcceptablePuyo = 6;
    if (attack &&
        attack->score >= SCORE_FOR_OJAMA * kAcceptablePuyo &&
        attack->score < plan.score()) {
      value = plan.score();
      oss << "Counter(" << value << ")_";
      score += value;
    }

    value = plan.rensaResult().score;
    oss << "Current(" << value << ")_";
    score += value;

    // Penalty for vanishments.
    value = -plan.totalFrames() * 10;
    oss << "Time(" << value << ")_";
    score += value;

    if (plan.field().countPuyos() == 0) {
      value = ZENKESHI_BONUS;
      oss << "Zenkeshi(" << value << ")";
      score += value;
    }
  } else {
    value = Evaluator::Plan(plan.field(), track);
    if (value) {
      oss << "Planning(" << value << ")_";
      score += value;
    }
  }

  LOG(INFO) << score << " " << oss.str();
  if (score > control->score) {
    control->score = score;
    control->message = oss.str();
    control->decision = plan.decisions().front();
  }
}

}  // namespace peria
