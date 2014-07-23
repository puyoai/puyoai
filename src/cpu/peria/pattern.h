#pragma once

#include <string>
#include <vector>

#include "core/field/core_field.h"

namespace peria {

class Pattern {
 public:
  static const std::vector<Pattern>& GetAllPattern();

  Pattern(const std::string& pattern);
  
  int Match(const CoreField& field) const;

 private:
  std::string pattern_;
};

}  // namespace peria


