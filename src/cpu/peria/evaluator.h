#pragma once

#include <string>

class CoreField;
class RefPlan;
class RensaChainTrackResult;
struct PlayerState;

namespace peria {

struct Control;
struct Player;

// Evaluator is a namespace to evaluate something.
class Evaluator {
 public:
  static void EvalPlan(const RefPlan& plan, const PlayerState& me, const Player& enemy, Control* control);
  
  static int PatternMatch(const CoreField& field, std::string* message);

  static int Field(const CoreField& field);

  static int Future(const CoreField& field);

  static int Plan(const CoreField& field, const RensaChainTrackResult& track);
};

}  // namespace peria
