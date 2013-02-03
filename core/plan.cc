#include "core/plan.h"

int Plan::numTurns() const {
  const Plan* p = this;
  int n = 0;
  while (p) {
    p = p->parent;
    n++;
  }
  return n;
}
