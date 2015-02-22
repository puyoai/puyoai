#pragma once

#include <deque>
#include <istream>
#include <map>
#include <set>
#include <string>

#include "core/core_field.h"

namespace peria {

class Pattern {
  typedef std::pair<char, char> Neighbor;

 public:
  typedef std::map<char, std::map<PuyoColor, int> > MatchingCounts;

  static void ReadBook(std::istream& is);
  static const std::vector<Pattern>& GetAllPattern();

  int Match(const CoreField& field) const;
  const std::string& name() const { return name_; }
  int score() const { return score_; }

 protected:
  bool ParseBook(std::istream& is);
  void Optimize();
  void AppendField(std::string line);
  int GetScore(MatchingCounts& matching) const;

  std::string name_;
  std::deque<std::string> pattern_;
  std::set<Neighbor> neighbors_;
  int score_ = 0;

  // Maxium number of Puyos to apply the pattern.
  int max_puyos_ = -1;

  // The number of Puyos to check in the pattern.
  int num_puyos_ = 0;
};

}  // namespace peria
