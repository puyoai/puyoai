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
  std::ostringstream oss;
  oss << "_";

  score += PatternMatch(field, message);
  score += Field(field);

  {  // Evaluate possible rensa.
    int value = Future(field);
    if (value > 0) {
      oss << "Future(" << value << ")_";
      score += value;
    }
  }

  *message += oss.str();
  return score;
}

int Evaluator::EvalRensa(const RefPlan& plan, std::string* message) {
  if (!plan.isRensaPlan())
    return 0;

  // Evaluate rensa
  // o TSUBUSHI : how quickly rensa ends. 2 or more lines.
  // - TAIOU : how small rensa is firable.
  int score = 0;
  std::ostringstream oss;

  {  // Basic rensa plan
    int value = plan.score() + (me.hasZenkeshi ? ZENKESHI_BONUS : 0);
    if (value > 0) {
      oss << "Rensa(" << value << ")_";
      score += value;
    }
  }

  {  // will be Zenkeshi
    int value = (plan.field().countPuyos() == 0) ? ZENKESHI_BONUS : 0;
    if (value > 0) {
      oss << "Zenkeshi(" << value << ")_";
      score += value;
    }
  }

  {  // penalty for using too many puyos
    int remained_puyos = me.field.countPuyos() - plan.field().countPuyos();
    int wasted_puyos = std::max(remained_puyos - 4 * plan.chains() - 4, 0);
    int value = -200 * plan.chains() * plan.chains() * wasted_puyos;
    if (value < 0) {
      oss << "Waste(" << value << ")_";
      score += value;
    }
  }

  {  // TSUBUSHI : how quickly rensa ends. 2 or more lines.
    int value = EvalTsubushi(plan);
    if (value > 0) {
      oss << "Tsubushi(" << value << ")_";
      score += value;
    }
  }

  *message = oss.str();
  return score;
}

int Evaluator::EvalTsubushi(const RefPlan& plan) {
  int score = 0;

  static const int kLines = 2;
  if (enemy_hands.firables.size() == 0
      && plan.score() >= kLines * FieldConstant::WIDTH * SCORE_FOR_OJAMA
      && plan.chains() <= 2) {
    score += plan.score();
    if (plan.decisions().size() == 1)
      score *= 2;
  }

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
      [&expects](const RefPlan& plan) {
        if (plan.isRensaPlan())
          expects.push_back(plan.score());
      });
  int value = std::accumulate(expects.begin(), expects.end(), 0);
  if (expects.size() > 2) {
    value /= (expects.size() - 1);
  }
  return value;
}

}  // namespace peria
