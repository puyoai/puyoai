#pragma once

#include <string>

class CoreField;
class RefPlan;
class RensaChainTrackResult;
struct PlayerState;

namespace peria {

struct Control;
struct Player;

// Evaluator evaluates something.
class Evaluator {
 public:
  Evaluator(const PlayerState&, const Player&, Control*);
  void EvalPlan(const RefPlan& plan);
  
 private:
  int PatternMatch(const CoreField& field, std::string* message);
  int Field(const CoreField& field);
  int Future(const CoreField& field);
  int Plan(const CoreField& field, const RensaChainTrackResult& track);

  const PlayerState& me;
  const Player& enemy;
  Control* control;
};

}  // namespace peria
