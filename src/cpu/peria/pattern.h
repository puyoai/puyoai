#pragma once

#include <istream>
#include <string>
#include <vector>

#include "core/field/core_field.h"

namespace peria {

class Pattern {
 public:
  static void ReadBook(std::istream& is);
  static const std::vector<Pattern>& GetAllPattern();

  int Match(const CoreField& field) const;

 private:
  std::string name_;
  std::vector<std::string> pattern_;
  int score_;
  // Maxium number of Puyos to apply the pattern.
  int max_puyos_;
};

}  // namespace peria


