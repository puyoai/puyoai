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

Evaluator::Evaluator(int id, const PlayerState& m, const PlayerState& e, const PlayerHands& eh, Control* c) : frame_id(id), me(m), enemy(e), enemy_hands(eh), control(c) {
  UNUSED_VARIABLE(frame_id);
}

void Evaluator::EvalPlan(const RefPlan& plan) {
  std::string message_field;
  std::string message_rensa;
  std::string message_time;

  int score = EvalField(plan.field(), &message_field);
  score += EvalRensa(plan, &message_rensa);
  score += EvalTime(plan, &message_time);

  std::string message;
  message += "Field : " + message_field;
  message += ",Rensa : " + message_rensa;
  message += ",Time : " + message_time;

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

  if (true) {
    std::string name;
    int value = PatternMatch(field, &name);
    if (value > 0) {
      oss << "Pattern(" << name << ")_";
      score += value;
    }
  }

  if (true) {
    int value = Flat(field);
    oss << "Flat(" << value << ")_";
    score += value;
  }

  if (true) {  // Evaluate possible rensa.
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

  if (false) {  // penalty for using too many puyos
    int remained_puyos = me.field.countPuyos() - plan.field().countPuyos();
    int wasted_puyos = std::max(remained_puyos - 4 * plan.chains() - 4, 0);
    int value = -200 * plan.chains() * plan.chains() * wasted_puyos;
    if (value < 0) {
      oss << "Waste(" << value << ")_";
      score += value;
    }
  }

  if (false) {  // TSUBUSHI : how quickly rensa ends. 2 or more lines.
    int value = EvalTsubushi(plan);
    if (value > 0) {
      oss << "Tsubushi(" << value << ")_";
      score += value;
    }
  }

  if (false) {  // TAIOU : how smartly (need definition)
    ;
  }

  *message = oss.str();
  return score;
}

int Evaluator::EvalTime(const RefPlan& plan, std::string* message) {
  // Evaluate time to control puyos and rensa.
  int score = 0;
  std::ostringstream oss;

  if (true) {  // Penalty : Time to set puyos, including time for rensa.
    int value = -plan.totalFrames();
    oss << "Base(" << value << ")_";
    score += value;
  }

  if (true) { // TAIOU : If enemy's rensa is going, fire my rensa in time.
    int value = 0;
    if (enemy.isRensaOngoing() && plan.isRensaPlan() && me.totalOjama(enemy) * 70 < plan.score())
      value = plan.score();
    if (value > 0) {
      oss << "Taiou(" << value << ")_";
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
  // TODO: Apply some templates' combinations.
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

int Evaluator::Flat(const CoreField& field) {
  const int kEdgePenalty = 40;
  const int kPenalty = 10;

  // Field should be like 'U'
  int diff12 = std::max(field.height(2) - field.height(1), 0);
  int diff23 = std::max(field.height(3) - field.height(2), 0);
  int diff34 = std::abs(field.height(3) - field.height(4));
  int diff45 = std::max(field.height(4) - field.height(5), 0);
  int diff56 = std::max(field.height(5) - field.height(6), 0);

  int score =
    - diff12 * std::max(field.height(2), field.height(1)) * kEdgePenalty
    - diff23 * std::max(field.height(3), field.height(2)) * kPenalty
    - diff34 * std::max(field.height(3), field.height(4)) * kPenalty
    - diff45 * std::max(field.height(4), field.height(5)) * kPenalty
    - diff56 * std::max(field.height(5), field.height(6)) * kEdgePenalty;

  return score;
}

int Evaluator::Future(const CoreField& field) {
  // |expects[i]| figures the maximum point of the RENSA, which
  // can be fired with |i| additional puyos.
  std::vector<int> expects(10, -1);

  RensaDetector::detectIteratively(
      field, RensaDetectorStrategy::defaultDropStrategy(), 1,
      [&expects](CoreField&& cf, const ColumnPuyoList& puyo_to_add) -> RensaResult {
        size_t num = puyo_to_add.size();
        RensaResult result = cf.simulate();
        if (num < expects.size()) {
          expects[num] = std::max(expects[num], result.score);
        }
        return result;
      });

  for (size_t i = 0; i < expects.size(); ++i) {
    if (expects[i] >= 0)
      return expects[i];
  }
  return 0;
}

}  // namespace peria
