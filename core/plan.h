#ifndef CORE_PLAN_H_
#define CORE_PLAN_H_

#include "core/decision.h"
#include "core/field.h"

class Plan {
 public:
  int numTurns() const;

  // Previous state.
  const Plan* parent;
  // Decision made for this plan.
  Decision decision;
  // Future field state (apply the state to the previous state).
  Field field;
  // Score we get with the future field.
  int score;
  // The number of Ojama puyos in Yokoku puyo.
  int ojama;
};

inline int Plan::numTurns() const {
  const Plan* p = this;
  int n = 0;
  while (p) {
    p = p->parent;
    n++;
  }
  return n;
}

#endif  // CORE_PLAN_H_
