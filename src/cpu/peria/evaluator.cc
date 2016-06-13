#include "evaluator.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <numeric>
#include <sstream>

#include "core/plan/plan.h"
#include "core/rensa/rensa_detector.h"
#include "core/kumipuyo_seq.h"
#include "cpu/peria/control.h"
#include "cpu/peria/evaluator.h"
#include "cpu/peria/pattern.h"
#include "cpu/peria/player_hands.h"

namespace peria {

Evaluator::Evaluator(const PlayerState& m, const PlayerState& e, const PlayerHands& eh, Control* c)
    : me(m), enemy(e), enemy_hands(eh), control(c) {}

void Evaluator::EvalPlan(const CoreField& field, int sending_ojama, const RefPlan& plan) {
  Genre genres[] = {
    {"Field", 0, ""},
    {"Rensa", 0, ""},
  };

  genres[0].score = EvalField(field, &genres[0].message);
  genres[1].score = EvalRensa(field, plan, sending_ojama, &genres[1].message);

  std::ostringstream oss;
  int score = 0;
  bool first = true;
  for (const auto& genre : genres) {
    if (!first)
      oss << "\n";
    oss << genre.name << "[" << genre.score << "]: " << genre.message;
    score += genre.score;
    first = false;
  }

  if (control->score < score) {
    control->decision = decision_;
    control->score = score;
    control->message = oss.str();
  }
}

int Evaluator::EvalField(const CoreField& field, std::string* message) {
  // Evaluate field
  // o pattern matching
  // o possible future rensa

  int score = 0;
  std::ostringstream oss;

  {
    std::string name;
    int value = DynamicPatternBook::iteratePatterns(field, &name);
    if (value > 0) {
      oss << "Pattern(" << name << ")_";
      score += value;
    }
  }

  if (false) {
    int value = Valley(field);
    oss << "Valley(" << value << ")_";
    score += value;
  }

  if (false) {  // Evaluate possible rensa.
    int value = Future(field) / 10;
    oss << "Future(" << value << ")_";
    score += value;
  }

  *message += oss.str();
  return score;
}

int Evaluator::EvalRensa(const CoreField& field, const RefPlan& plan, int sending_ojama, std::string* message) {
  if (sending_ojama < 0)
    return 0;

  // Evaluate rensa
  // o TSUBUSHI : how quickly rensa ends. 2 or more lines.
  // - TAIOU : how small rensa is firable.
  int score = 0;
  std::ostringstream oss;

  if (true) {  // Basic rensa plan
    int value = sending_ojama;
    if (value > 0) {
      oss << "Rensa(" << value << ")_";
      score += value;
    }
  }

  if (false) {  // penalty for using too many puyos
    int remained_puyos = me.field.countPuyos() - field.countPuyos();
    int wasted_puyos = std::max(remained_puyos - 4 * plan.chains() - 4, 0);
    int value = -5 * plan.chains() * wasted_puyos;
    if (value < 0) {
      oss << "Waste(" << value << ")_";
      score += value;
    }
  }

  if (false) {  // bonus to use many puyos, only for the main fire
    int value = 0;
    if (me.field.countPuyos() > field.countPuyos() * 2) {
      value = plan.score() / 500;
    }
    if (value > 0) {
      oss << "UseAll(" << value << ")_";
      score += value;
    }
  }

  // TODO: Tsubushij
  if (true) {  // TSUBUSHI : how quickly rensa ends. 2 or more lines.
    int value = EvalTsubushi(plan);
    if (value > 0) {
      oss << "Tsubushi(" << value << ")_";
      score += value;
    }
  }

  // TODO: Taiou

  // TODO: use this feature to judge if I should fire the main chain.
  if (false) {  // penalty for enemy's plans
    int value = EvalEnemyPlan();
    if (value < 0) {
      oss << "Enemy(" << value << ")_";
      score += value;
    }
  }

  *message = oss.str();
  return score;
}

int Evaluator::EvalTsubushi(const RefPlan& plan) {
  int score = 0;

  int lines = 2;
  if (enemy.field.height(3) > 9)
    lines = 13 - enemy.field.height(3);

  if (enemy_hands.firables.size() == 0
      && plan.score() >= lines * FieldConstant::WIDTH * SCORE_FOR_OJAMA
      && plan.chains() <= 2) {
    score += plan.score();
    if (plan.decisions().size() == 1)
      score *= 2;
  }

  return score;
}

int Evaluator::EvalEnemyPlan() {
  PossibleRensa max_firable;
  for (const auto& firable : enemy_hands.firables) {
    if (max_firable.score < firable.score)
      max_firable = firable;
  }

  PossibleRensa max_possible;
  for (const auto& possible : enemy_hands.need_keys) {
    if (max_possible.eval() < possible.eval())
      max_possible = possible;
  }

  return -std::max(max_firable.score * 0.5, max_possible.score * 0.3);
}

int Evaluator::Valley(const CoreField& field) {
  int score = 0;

  const int kEdgePenalty = 40;
  const int kPenalty = 10;

  // Field should be like 'U'
  int diff12 = std::max(field.height(2) - field.height(1), 0);
  int diff23 = std::max(field.height(3) - field.height(2), 0);
  int diff34 = std::abs(field.height(3) - field.height(4));
  int diff45 = std::max(field.height(4) - field.height(5), 0);
  int diff56 = std::max(field.height(5) - field.height(6), 0);

  score -= diff12 * std::max(field.height(2), field.height(1)) * kEdgePenalty
      + diff23 * std::max(field.height(3), field.height(2)) * kPenalty
      + diff34 * std::max(field.height(3), field.height(4)) * kPenalty
      + diff45 * std::max(field.height(4), field.height(5)) * kPenalty
      + diff56 * std::max(field.height(5), field.height(6)) * kEdgePenalty;

  // If (3, 11) is filled, add a large penalty for 1/6 risk.
  if (!field.isEmpty(FieldConstant::WIDTH / 2, FieldConstant::HEIGHT - 1))
    score -= 1000;

  return score;
}

int Evaluator::Future(const CoreField& field) {
  UNUSED_VARIABLE(field);
  std::vector<int> scores;
  if (field.countPuyos() < 60) {
    Plan::iterateAvailablePlans(
        field, KumipuyoSeq(), 1,
        [&scores](const RefPlan& plan) {
          if (plan.isRensaPlan())
            scores.push_back(plan.rensaResult().score);
        });
  }
  return std::accumulate(scores.begin(), scores.end(), 0);
}

}  // namespace peria
