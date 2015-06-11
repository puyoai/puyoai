#include "evaluator.h"

#include <numeric>
#include <sstream>

#include "core/algorithm/plan.h"
#include "core/algorithm/rensa_detector.h"
#include "core/kumipuyo_seq.h"
#include "cpu/peria/control.h"
#include "cpu/peria/evaluator.h"
#include "cpu/peria/pattern.h"
#include "cpu/peria/player_hands.h"

namespace peria {

int ScoreDiffHeight(int higher, int lower) {
  int diff = higher - lower;
  diff = diff * diff;
  return ((higher > lower) ? 0 : -diff) * 50;
}

Evaluator::Evaluator(const PlayerState& m, const PlayerState& e, const PlayerHands& eh, Control* c)
  : me(m), enemy(e), enemy_hands(eh), control(c) {
  UNUSED_VARIABLE(me);
  UNUSED_VARIABLE(enemy);
  UNUSED_VARIABLE(enemy_hands);
}

void Evaluator::EvalPlan(const RefPlan& plan) {
  std::string message_field;
  std::string message_rensa;

  int score = EvalField(plan.field(), &message_field);
  score += EvalRensa(plan, &message_rensa);

  std::string message;
  message += "Field : " + message_field;
  if (!message_rensa.empty())
    message += ",Rensa : " + message_rensa;
  
  if (control->score < score) {
    control->decision = plan.decisions().front();
    control->score = score;
    control->message = message;
  }
}

int Evaluator::EvalField(const CoreField& field, std::string* message) {
  // Evaluate field
  // o pattern matching
  // o possible future rensa
  // - UKE : assume 1, 2 lines of ojama. 5 lines in case of ZENKESHI, and eval again.

  int score = 0;
  score += PatternMatch(field, message);
  score += Field(field);
  score += Future(field);
  return score;
}

int Evaluator::EvalRensa(const RefPlan& plan, std::string* message) {
  // Evaluate rensa
  // - TSUBUSHI : how quickly rensa ends. 2 or more lines.
  // - TAIOU : how small rensa is firable.

  int score = 0;
  score += plan.score();
  std::ostringstream oss;
  oss << "Rensa(" << score << ")";
  *message = oss.str();
  return score;
}

int Evaluator::PatternMatch(const CoreField& field, std::string* name) {
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

int Evaluator::Field(const CoreField& field) {
  int score = 0;

  int num_connect = 0;
  for (int x = 1; x < FieldConstant::WIDTH; ++x) {
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

int Evaluator::Future(const CoreField& field) {
  std::vector<int> expects;
  Plan::iterateAvailablePlans(
      field, KumipuyoSeq(), 1,
      [&expects](const RefPlan& p) {
        if (p.isRensaPlan())
          expects.push_back(p.rensaResult().score);
      });
  int value = std::accumulate(expects.begin(), expects.end(), 0);
  if (expects.size()) {
    value /= expects.size();
  }
  return value;
}

}  // namespace peria
