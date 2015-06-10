#pragma once

#include <string>

class Decision;

namespace peria {

struct Control {
  Decision decision;
  std::string message;
  int score = 0;
};

}  // namespace peria
