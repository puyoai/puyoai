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

Evaluator::Evaluator(int id, const PlayerState& m, const PlayerState& e, const PlayerHands& eh, Control* c) : frame_id(id), me(m), enemy(e), enemy_hands(eh), control(c) {
  UNUSED_VARIABLE(frame_id);
}

void Evaluator::EvalPlan(const RefPlan& plan) {
  Genre genres[] = {
    {"Field", 0, ""},
    {"Uke", 0, ""},
    {"Rensa", 0, ""},
    {"Time", 0, ""}};

  genres[0].score = EvalField(plan.field(), &genres[0].message);
  genres[1].score = EvalUke(plan.field(), &genres[1].message);
  genres[2].score = EvalRensa(plan, &genres[2].message);
  genres[3].score = EvalTime(plan, &genres[3].message);

  std::ostringstream oss;
  int score = 0;
  for (const auto& genre : genres) {
    oss << "," << genre.name << "[" << genre.score << "]: " << genre.message;
    score += genre.score;
  }

  // TODO: Use better randomness.
  if (control->score < score - 70 || std::exp(control->score - score) < rand() * (1.0 / 0x7fffffff)) {
    control->decision = plan.decisions().front();
    control->score = score;
    control->message = oss.str().substr(1);  // remove first ','
  }
}

int Evaluator::EvalField(const CoreField& field, std::string* message) {
  // Evaluate field
  // o pattern matching
  // o possible future rensa

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
    int value = Valley(field);
    oss << "Valley(" << value << ")_";
    score += value;
  }

  if (true) {  // Evaluate possible rensa.
    int value = Future(field) / 10;
    oss << "Future(" << value << ")_";
    score += value;
  }

  *message += oss.str();
  return score;
}

int Evaluator::EvalUke(const CoreField& field, std::string* message) {
  // Evaluate field assuming some Ojama puyo fall.
  // x possible future rensa
  // x counter for zenkeshi

  int score = 0;
  std::ostringstream oss;

  if (false) {
    CoreField cf = field;
    // Assume 1 or 5 line(s) of Ojama puyo
    cf.fallOjama(enemy.hasZenkeshi ? 5 : 1);
    int value = Future(cf) / 20;
    oss << "Future(" << value << ")_";
    score += value;
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

  if (true) {  // Basic rensa plan
    int value = plan.score() + (me.hasZenkeshi ? ZENKESHI_BONUS : 0);
    if (value > 0) {
      oss << "Rensa(" << value << ")_";
      score += value;
    }
  }

  if (false) {  // Evaluate possible rensa.
    int value = Future(plan.field()) / 100;
    oss << "Future(" << value << ")_";
    score += value;
  }

  if (false) {  // penalty for using too many puyos
    int remained_puyos = me.field.countPuyos() - plan.field().countPuyos();
    int wasted_puyos = std::max(remained_puyos - 4 * plan.chains() - 4, 0);
    int value = -5 * plan.chains() * wasted_puyos;
    if (value < 0) {
      oss << "Waste(" << value << ")_";
      score += value;
    }
  }

  if (false) {  // bonus to use many puyos, only for the main fire
    int value = 0;
    if (me.field.countPuyos() > plan.field().countPuyos() * 2) {
      value = plan.score() / 500;
    }
    if (value > 0) {
      oss << "UseAll(" << value << ")_";
      score += value;
    }
  }

  if (true) {  // TSUBUSHI : how quickly rensa ends. 2 or more lines.
    int value = EvalTsubushi(plan);
    if (value > 0) {
      oss << "Tsubushi(" << value << ")_";
      score += value;
    }
  }

  if (false) {  // TAIOU : how smartly (need definition)
    ;
  }

  // TODO: use this feature to judge if I should fire the main chain.
  if (false) {  // penalty for enemy's plans
    int value = EvalEnemyPlan();
    if (value < 0) {
      oss << "Enemy(" << value << ")_";
      score += value;
    }
  }

  if (false) {  // 2-double
    const RensaResult& result = plan.rensaResult();
    bool is_2dub = (result.chains == 2 && result.score >= 920);
    if (is_2dub) {
      oss << "NiDub(" << result.score << ")_";
      score += result.score * 2;
    }
  }

  *message = oss.str();
  return score;
}

int Evaluator::EvalTime(const RefPlan& plan, std::string* message) {
  // Evaluate time to control puyos and rensa.
  int score = 0;
  std::ostringstream oss;

  if (false) {  // Penalty : Time to set puyos, including time for rensa.
    int value = -plan.totalFrames();
    oss << "Base(" << value << ")_";
    score += value;
  }

  if (false) { // TAIOU : If enemy's rensa is going, fire my rensa in time.
    int value = 0;
    if (enemy.isRensaOngoing() && plan.isRensaPlan() && me.totalOjama(enemy) * 70 < plan.score())
      value = plan.score();
    if (value > 0) {
      oss << "Taiou(" << value << "-->" << enemy.currentRensaResult.score << ")_";
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
