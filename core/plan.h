#ifndef CORE_PLAN_H_
#define CORE_PLAN_H_

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

#endif  // CORE_PLAN_H_
