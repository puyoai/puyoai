#ifndef CORE_PLAN_H_
#define CORE_PLAN_H_

#include <vector>

#include "core/ctrl.h"
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

inline void FindAvailablePlansInternal(
    const Field& field, const Plan* parent, int depth, int max_depth,
    std::vector<Plan>* plans) {

  static const Decision decisions[] = {
    Decision(1, 2),
    Decision(2, 2),
    Decision(3, 2),
    Decision(4, 2),
    Decision(5, 2),
    Decision(6, 2),
    Decision(2, 3),
    Decision(3, 3),
    Decision(4, 3),
    Decision(5, 3),
    Decision(6, 3),

    Decision(1, 0),
    Decision(2, 0),
    Decision(3, 0),
    Decision(4, 0),
    Decision(5, 0),
    Decision(6, 0),
    Decision(1, 1),
    Decision(2, 1),
    Decision(3, 1),
    Decision(4, 1),
    Decision(5, 1),
  };

  char c1 = field.GetNextPuyo(depth * 2 + 0);
  char c2 = field.GetNextPuyo(depth * 2 + 1);
  int num_decisions = (c1 == c2) ? 11 : 22;

  // Cause of slowness?
  int heights[Field::MAP_WIDTH+1];
  for (int x = 1; x <= Field::WIDTH; x++) {
    heights[x] = 100;
    for (int y = 1; y <= Field::HEIGHT + 2; y++) {
      if (field.Get(x, y) == EMPTY) {
        heights[x] = y;
        break;
      }
    }
  }

  for (int i = 0; i < num_decisions; i++) {
    const Decision& decision = decisions[i];
    if (!Ctrl::isReachable(field, decision)) {
      continue;
    }

    Field next_field(field);

    int x1 = decision.x;
    int x2 = decision.x + (decision.r == 1) - (decision.r == 3);

    if (decision.r == 2) {
      next_field.Set(x2, heights[x2]++, c2);
      next_field.Set(x1, heights[x1]++, c1);
    } else {
      next_field.Set(x1, heights[x1]++, c1);
      next_field.Set(x2, heights[x2]++, c2);
    }
    heights[x1]--;
    heights[x2]--;
    int chains, score, frames;
    next_field.Simulate(&chains, &score, &frames);
    if (next_field.Get(3, 12) != EMPTY) {
      continue;
    }

    // Add a new plan.
    plans->push_back(Plan());
    Plan& plan = plans->at(plans->size() - 1);
    plan.field = next_field;
    plan.decision = decision;
    plan.parent = parent;
    if (parent) {
      plan.score = parent->score + score;
    } else {
      plan.score = score;
    }

    if (depth < max_depth - 1) {
      FindAvailablePlansInternal(next_field, &plan, depth + 1, max_depth, plans);
    }
  }
}

inline void FindAvailablePlans(const Field& field, int depth, std::vector<Plan>* plans) {
  plans->clear();
  plans->reserve(22 + 22*22 + 22*22*22);
  FindAvailablePlansInternal(field, NULL, 0, depth, plans);
}

inline void FindAvailablePlans(const Field& field, std::vector<Plan>* plans) {
  FindAvailablePlans(field, 3, plans);
}

#endif  // CORE_PLAN_H_
