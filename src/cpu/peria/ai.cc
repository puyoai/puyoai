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
#include "core/core_field.h"
#include "core/frame_request.h"
#include "core/player_state.h"

#include "cpu/peria/evaluator.h"

namespace peria {

struct Ai::Attack {
  int score;
  int end_frame_id;
};

struct Ai::Control {
  std::string message;
  int score = 0;
  CoreField field;
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
  auto evaluate = std::bind(Ai::EvaluatePlan, _1, me, enemy, track_result, &control);
  Plan::iterateAvailablePlans(field, seq, 2, evaluate);

  // Simulate plan to see where to start.
  Position start(0, 0);
  auto callback = [&start](CoreField&& cf, const ColumnPuyoList&) -> RensaResult {
    RensaChainTracker tracker;
    RensaResult rensa_result = cf.simulate(&tracker);
    for (int x = 1; x <= FieldConstant::WIDTH; ++x) {
      for (int y = 1; y <= FieldConstant::HEIGHT; ++y) {
        if (tracker.result().erasedAt(x, y) == 1) {
          start = Position(x, y);
          break;
        }
      }
    }

    return rensa_result;
  };
  RensaDetector::detectIteratively(
      control.field, RensaDetectorStrategy::defaultDropStrategy(), 1, callback);

  if (start.x) {
    std::ostringstream oss;
    oss << "," << "StartFrom(" << toChar(field.color(start.x, start.y))
        << "@" << start.x << "-" << start.y << ")";
    control.message += oss.str();
  }

  return DropDecision(control.decision, control.message);
}

void Ai::EvaluatePlan(const RefPlan& plan,
                      const PlayerState& me,
                      const PlayerState& enemy,
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

  int num_puyos_to_fire = 0;
  // TODO: Move this routine into Evaluator::Field()
  // Simulate plan to see where to start.
  int start_height = 0;
  auto callback = [&start_height, &num_puyos_to_fire](
      CoreField&& cf, const ColumnPuyoList& completedPuyos) -> RensaResult {
    RensaChainTracker tracker;
    RensaResult rensa_result = cf.simulate(&tracker);
    int num_fire = 0;
    for (int x = 1; x <= FieldConstant::WIDTH; ++x) {
      for (int y = 1; y <= FieldConstant::HEIGHT; ++y) {
        if (tracker.result().erasedAt(x, y) == 1) {
          start_height += y;
          ++num_fire;
        }
      }
    }
    if (num_fire)
      start_height /= num_fire;
    num_puyos_to_fire = completedPuyos.size();
    return rensa_result;
  };
  RensaDetector::detectIteratively(
      plan.field(), RensaDetectorStrategy::defaultDropStrategy(), 1, callback);
  {
    value = start_height * 100;
    oss << "Starting(" << value << ")_";
    score += value;
  }
  {
    value = - (num_puyos_to_fire - 1) * 400;
    oss << "NeedToFire(" << value << ")_";
    score += value;
  }

  if (plan.isRensaPlan()) {
    const int kAcceptablePuyo = 6;
    int my_score = plan.rensaResult().score;
    if (me.hasZenkeshi)
      my_score += SCORE_FOR_OJAMA * ZENKESHI_BONUS;

    if (enemy.isRensaOngoing() &&
        enemy.currentRensaResult.score >= SCORE_FOR_OJAMA * kAcceptablePuyo &&
        enemy.currentRensaResult.score < my_score) {
      value = plan.score();
      oss << "Counter(" << value << ")_";
      score += value;
    }

    value = my_score;
    oss << "Current(" << value << ")_";
    score += value;

    // Penalty for vanishments.
    value = -plan.totalFrames() * 10;
    oss << "Time(" << value << ")_";
    score += value;

    if (plan.field().countPuyos() == 0) {
      value = ZENKESHI_BONUS;
      oss << "Zenkeshi(" << value << ")_";
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
    control->field = plan.field();
    control->score = score;
    control->message = oss.str();
    control->decision = plan.decisions().front();
  }
}

}  // namespace peria
