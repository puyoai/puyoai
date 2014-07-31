#pragma once

#include <istream>
#include <map>
#include <string>
#include <deque>

#include "core/field/core_field.h"

namespace peria {

class Pattern {
 public:
  static void ReadBook(std::istream& is);
  static const std::vector<Pattern>& GetAllPattern();

  int Match(const CoreField& field) const;
  const std::string& name() const { return name_; }

 private:
  void AppendField(std::string line);
  int GetScore(std::map<char, std::map<PuyoColor, int> >& matching) const;

  std::string name_;
  std::deque<std::string> pattern_;
  int score_;
  // Maxium number of Puyos to apply the pattern.
  int max_puyos_;
  // The number of Puyos to check in the pattern.
  int num_puyos_ = 0;
};

}  // namespace peria


