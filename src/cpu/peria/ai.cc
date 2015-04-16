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

#include "cpu/peria/pattern.h"

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
                       const EnemyState& enemy,
                       bool fast) const {
  UNUSED_VARIABLE(frame_id);
  UNUSED_VARIABLE(me);
  UNUSED_VARIABLE(enemy);
  UNUSED_VARIABLE(fast);
  using namespace std::placeholders;

  // Detect currently planning rensa.
  RensaChainTrackResult track_result;
  int _max_score = 0;
  auto rensa_callback = std::bind(Ai::EvaluateRensa, _1, _2, _3, _4,
                                  &_max_score, &track_result);
  RensaDetector::iteratePossibleRensasIteratively(field, 1, RensaDetectorStrategy::defaultFloatStrategy(), rensa_callback);


  // Look for plans.
  Control control;
  control.score = -10000;
  auto evaluate = std::bind(Ai::Evaluate, _1, attack_.get(),
                            track_result, &control);
  Plan::iterateAvailablePlans(field, seq, 2, evaluate);

  DLOG(INFO) << control.message;
  DLOG(INFO) << field.toDebugString();
  return DropDecision(control.decision, control.message);
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

int Ai::PatternMatch(const RefPlan& plan, std::string* name) {
  const CoreField& field = plan.field();

  int best = 0;
  for (const Pattern& pattern : Pattern::GetAllPattern()) {
    int score = pattern.Match(field);
    if (score > best) {
      best = score;

      std::ostringstream oss;
      oss << pattern.name() << "[" << score << "/" << pattern.score() << "]";
      *name = oss.str();
    }
  }

  return best;
}

int ScoreDiffHeight(int higher, int lower) {
  int diff = higher - lower;
  diff = diff * diff;
  return ((higher > lower) ? 0 : -diff) * 50;
}

int FieldEvaluate(const CoreField& field) {
  int score = 0;

  int num_connect = 0;
  for (int x = 1; x < PlainField::WIDTH; ++x) {
    int height = field.height(x);
    for (int y = 1; y <= height; ++y) {
      PuyoColor c = field.color(x, y);
      if (c == PuyoColor::OJAMA)
        continue;
      if (c == field.color(x + 1, y))
        ++num_connect;
      if (c == field.color(x, y + 1))
        ++num_connect;
    }
  }
  score += num_connect * 2;

  score += ScoreDiffHeight(field.height(1), field.height(2));
  score += ScoreDiffHeight(field.height(2), field.height(3));
  score += ScoreDiffHeight(field.height(5), field.height(4));
  score += ScoreDiffHeight(field.height(6), field.height(5));

  return score;
}

void Ai::Evaluate(const RefPlan& plan,
                  Attack* attack,
                  const RensaChainTrackResult& track,
                  Control* control) {
  int score = 0;
  int value = 0;
  std::ostringstream oss;
  std::string message;

  // Near future expectation
  {
    std::vector<int> expects;
    expects.clear();
    Plan::iterateAvailablePlans(
        plan.field(), KumipuyoSeq(), 1,
        [&expects](const RefPlan& p) {
          if (p.isRensaPlan())
          expects.push_back(p.rensaResult().score);
        });
    value = std::accumulate(expects.begin(), expects.end(), 0);
    if (expects.size()) {
      value /= expects.size();
      oss << "Future(" << value << ")_";
      score += value;
    }
  }

  // Pattern maching
  {
    value = PatternMatch(plan, &message);
    if (value) {
      oss << "Pattern(" << message << "=" << value << ")_";
      score += value;
    }
  }

  // Expected future
  {
    using namespace std::placeholders;
    RensaChainTrackResult track_result;
    int max_score = 0;
    auto rensa_callback = std::bind(Ai::EvaluateRensa, _1, _2, _3, _4,
                                    &max_score, &track_result);
    RensaDetector::iteratePossibleRensasIteratively(
        plan.field(), 1, RensaDetectorStrategy::defaultFloatStrategy(),
        rensa_callback);
    int count = 0;
    for (int x = 1; x <= PlainField::WIDTH; ++x) {
      for (int y = 1; y <= PlainField::HEIGHT; ++y) {
        if (track_result.erasedAt(x, y) == 0)
          continue;
        if (track_result.erasedAt(x, y) == track.erasedAt(x, y) + 1)
          ++count;
      }
    }

    if (count) {
      int value = count * 20;
      oss << "Planning(" << value << ")_";
      score += value;
    }
  }

  value = FieldEvaluate(plan.field());
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
  }

  LOG(INFO) << score << " " << oss.str();
  if (score > control->score) {
    control->score = score;
    control->message = oss.str();
    control->decision = plan.decisions().front();
  }
}

void Ai::EvaluateRensa(const CoreField& field,
                       const RensaResult& result,
                       const ColumnPuyoList& list,
                       const RensaChainTrackResult& track,
                       int* max_score,
                       RensaChainTrackResult* track_result) {
  UNUSED_VARIABLE(field);
  UNUSED_VARIABLE(result);
  UNUSED_VARIABLE(list);

  if (result.score > *max_score) {
    *max_score = result.score;
    *track_result = track;
  }
}

}  // namespace peria
