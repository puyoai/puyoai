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
  void EvalPlan(const CoreField&, int sending_ojama, const RefPlan& plan);

  void setDecision(const Decision& decision) { decision_ = decision; }
  
 private:
  struct Genre {
    std::string name;
    int score;
    std::string message;
  };

  int EvalField(const CoreField& field, std::string* message);
  int EvalUke(const CoreField& field, std::string* message);
  int EvalRensa(const CoreField& field, const RefPlan& plan, int sending_ojama, std::string* message);

  // Field related eval functions
  int PatternMatch(const CoreField& field, std::string* message);
  int Valley(const CoreField& field);
  int Future(const CoreField& field);

  // Eval functions for Rensa.
  int EvalTsubushi(const RefPlan& plan);
  int EvalEnemyPlan();

  const PlayerState& me;
  const PlayerState& enemy;
  const PlayerHands& enemy_hands;
  Decision decision_;

  // output
  Control* control;
};

}  // namespace peria
