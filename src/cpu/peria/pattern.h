#pragma once

#include <istream>
#include <string>
#include <deque>

#include "core/field/core_field.h"

namespace peria {

class Pattern {
 public:
  static void ReadBook(std::istream& is);
  static const std::vector<Pattern>& GetAllPattern();

  int Match(const CoreField& field) const;

 private:
  void AppendField(std::string line);
  void set_score(int score) { score_ = score; }
  void set_max_puyos(int max_puyos) { max_puyos_ = max_puyos; }

  std::string name_;
  std::deque<std::string> pattern_;
  int score_;
  // Maxium number of Puyos to apply the pattern.
  int max_puyos_;
};

}  // namespace peria


