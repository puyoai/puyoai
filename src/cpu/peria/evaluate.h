#pragma once

#include <string>

class RefPlan;
class Decision;

namespace peria {

class Evaluate {
 public:
  static void Future(const RefPlan& plan, int* best_score);
  static void Usual(const RefPlan& plan, int* best_score, Decision* decision);
  static void Patterns(const RefPlan& plan,
                       int* best_score,
                       std::string* best_name,
                       int* frames,
                       Decision* decision);
  static void Counter(const RefPlan& plan,
                      int threshold,
                      int frame,
                      int* score,
                      Decision* decision);

 protected:
  static int PatternMatch(const RefPlan& plan, std::string* name);

};

}  // namespace peria
