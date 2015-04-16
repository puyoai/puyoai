#pragma once

#include <string>

class CoreField;
class RefPlan;
class RensaChainTrackResult;

namespace peria {

// Evaluator is a namespace to evaluate something.
class Evaluator {
 public:
  static int PatternMatch(const CoreField& field, std::string* message);

  static int Field(const CoreField& field);

  static int Future(const CoreField& field);

  static int Plan(const CoreField& field,
                  const RensaChainTrackResult& track);
};

}  // namespace peria
