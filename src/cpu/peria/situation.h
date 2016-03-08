#pragma once

#include "core/decision.h"
#include "core/core_field.h"

class KumipuyoSeq;

namespace peria {

class Situation {
public:
  Situation(const Decision& firstDecision, const CoreField& field,
            const int score, const int frameId);
  ~Situation();
  void evaluate(const KumipuyoSeq& seq);

  const Decision& decision() { return decision_; }
  double avgScore() { return avgScore_; }
  double ucb(int n) const;
  
 private:
  void addScore(int value);

  Decision decision_;

  CoreField field_;
  int score_;  // score gotten in 2nd hand
  int frameId_;
  
  int numTried_ = 0;
  int sumScore_ = 0;
  double avgScore_ = 0;
};

}  // namespace peria
