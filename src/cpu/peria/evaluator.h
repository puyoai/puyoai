#pragma once

#include <string>
#include "core/decision.h"

class CoreField;
class RefPlan;
class RensaChainTrackResult;
struct PlayerState;

namespace peria {

struct Control;
struct PlayerHands;

// Evaluator evaluates something.
class Evaluator {
 public:
  Evaluator(const PlayerState& me, const PlayerState& enemy, const PlayerHands&, Control*);
  void EvalPlan(const PlayerState& me, const PlayerState& enemy, const RefPlan& plan);

  // TODO: remove this method.
  void setDecision(const Decision& decision) { decision_ = decision; }

 private:
  struct Genre {
    std::string name;
    int score;
    std::string message;

    Genre(const std::string& n) : name(n), score(0) {}
  };

  Genre EvalField(const PlayerState& me, const PlayerState& enemy, const RefPlan& plan);
  Genre EvalRensa(const PlayerState& me, const PlayerState& enemy, const RefPlan& plan);

  // Field related eval functions
  int PatternMatch(const CoreField& field, std::string* message);
  int Valley(const CoreField& field);
  int Future(const CoreField& field);

  // Eval functions for Rensa.
  int EvalTsubushi(const PlayerState& me, const PlayerState& enemy);
  int EvalEnemyPlan();

  const PlayerState& me_from;
  const PlayerState& enemy_from;
  const PlayerHands& enemy_hands;
  Decision decision_;

  // output
  Control* control;
};

}  // namespace peria
