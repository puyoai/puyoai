#include "cpu/peria/ai.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <climits>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "core/algorithm/plan.h"
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
                       const PlayerState& enemy,
                       bool fast) const {
  UNUSED_VARIABLE(frame_id);
  UNUSED_VARIABLE(me);
  UNUSED_VARIABLE(enemy);
  UNUSED_VARIABLE(fast);
  using namespace std::placeholders;

  Control control;
  auto evaluate = std::bind(Ai::Evaluate, _1, attack_.get(), &control);
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
  int sum = 0;
  int best = 0;

  const CoreField& field = plan.field();
  LOG(INFO) << field.toDebugString();

  std::ostringstream oss;
  oss << "Puyo.count:" << field.countPuyos() << "_";
  for (const Pattern& pattern : Pattern::GetAllPattern()) {
    int score = pattern.Match(field);
    sum += score;
    if (score > best) {
      best = score;
    }
    oss << pattern.name() << "[" << score << "/" << pattern.score() << "]";
  }
  *name = oss.str();

  return sum;
}

int FieldEvaluate(const CoreField& field) {
#if 0
  return 0;
#else
  int num_connect = 0;
  for (int x = 1; x < PlainField::WIDTH; ++x) {
    int height = field.height(x);
    for (int y = 1; y <= height; ++y) {
      PuyoColor c = field.color(x, y);
      if (c == PuyoColor::OJAMA)
        continue;
      if (c == field.color(x + 1, y))
        num_connect += 2;
      if (c == field.color(x, y + 1))
        num_connect += 3;
    }
  }
  return num_connect;
#endif
}

void Ai::Evaluate(const RefPlan& plan, Attack* attack, Control* control) {
  int score = 0;
  int value = 0;
  std::ostringstream oss;
  std::string message;

  UNUSED_VARIABLE(attack);

#if 0  // Futreu expectation
  std::vector<int> expects;
  expects.clear();
  Plan::iterateAvailablePlans(
      plan.field(), KumipuyoSeq(), 1,
      [&expects](const RefPlan& p) {
        if (p.isRensaPlan())
          expects.push_back(p.rensaResult().score);
      });
  int expect = std::accumulate(expects.begin(), expects.end(), 0);
  if (expects.size())
    expect /= expects.size();
#endif

#if 1  // Pattern maching
  value = PatternMatch(plan, &message);
  oss << "Pattern(" << message << "," << value << ")_";
  score += value;
#endif

#if 0
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

    value = expect;
    oss << "Future(" << value << ")";
    score += value;

    if (plan.field().countPuyos() == 0) {
      value = 9999;
      oss << "Zenkeshi(" << value << ")";
      score += value;
    }
  } else {
    value = expect;
    oss << "Future(" << value << ")";
    score += value;

    value = FieldEvaluate(plan.field());
    oss << "Field(" << value << ")";
    score += value;
  }
#endif

  LOG(INFO) << score << " " << oss.str();
  if (score > control->score) {
    control->score = score;
    control->message = oss.str();
    control->decision = plan.decisions().front();
  }
}

}  // namespace peria
