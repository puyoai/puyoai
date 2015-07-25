#pragma once

#include <limits>
#include <string>

class Decision;

namespace peria {

struct Control {
  Decision decision;
  std::string message;
  int score = std::numeric_limits<int>::min();
};

}  // namespace peria
