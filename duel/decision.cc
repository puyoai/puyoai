#include "decision.h"

bool Decision::IsValid() const {
  if (x == 1 && r == 3) {
    return false;
  }
  if (x == 6 && r == 1) {
    return false;
  }
  return (1 <= x && x <= 6 && 0 <= r && r <= 3);
}

bool Decision::operator == (const Decision& d) const {
  return (this->x == d.x) && (this->r == d.r);
}

const Decision Decision::NO_INPUT(-1, -1);
const Decision Decision::USE_LAST_INPUT(0, 0);
