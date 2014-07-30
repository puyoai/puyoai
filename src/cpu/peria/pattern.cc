#include "pattern.h"

#include <cctype>
#include <deque>
#include <istream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace peria {

namespace {

std::vector<Pattern> g_pattern;

std::string Trim(std::string line) {
  size_t pos = line.find('#');
  // Remove comments starting with '#'.
  if (pos != std::string::npos)
    line = line.substr(0, pos);
  size_t start = line.find_first_not_of(' ');
  size_t last = line.find_last_not_of(' ');
  if (start == std::string::npos)
    start = 0;
  if (last == std::string::npos)
    last = line.size();
  return line.substr(start, last - start + 1);
}

}  // namespace

void Pattern::ReadBook(std::istream& is) {
  g_pattern.clear();

  std::unique_ptr<Pattern> pattern(new Pattern);
  for (std::string line; std::getline(is, line);) {
    line = Trim(line);
    if (line.empty())
      continue;
    std::istringstream iss(line);
    std::string first_segment;
    iss >> first_segment;
    if (first_segment.find(':') == std::string::npos) {
      pattern->AppendField(first_segment);
      continue;
    }

    if (first_segment == "NAME:") {
      if (pattern)
        g_pattern.push_back(*pattern);
      pattern.reset(new Pattern);
      iss >> pattern->name_;
    } else if (first_segment == "SCORE:") {
      int score;
      iss >> score;
      pattern->score_ = score;
    } else if (first_segment == "MAX:") {
      int max_puyos;
      iss >> max_puyos;
      pattern->max_puyos_ = max_puyos;
    }
  }

  if (pattern)
    g_pattern.push_back(*pattern);
}

const std::vector<Pattern>& Pattern::GetAllPattern() {
  return g_pattern;
}

int Pattern::Match(const CoreField& field) const {
  if (field.countPuyos() > max_puyos_)
    return 0;

  std::map<char, std::map<PuyoColor, int> > matching0;
  std::map<char, std::map<PuyoColor, int> > matching1;  // Mirroring
  for (int x = 1; x <= PlainField::WIDTH; ++x) {
    for (size_t y = 1; y <= pattern_.size(); ++y) {
      const char c0 = pattern_[y - 1][x - 1];
      const char c1 = pattern_[y - 1][PlainField::WIDTH - x];
      if (c0 != '.')
        ++matching0[c0][field.get(x, y)];
      if (c1 != '.')
        ++matching1[c1][field.get(x, y)];
    }
  }
  
  return std::max(GetScore(matching0), GetScore(matching1));
}

void Pattern::AppendField(std::string line) {
  DCHECK(line.size() == PlainField::WIDTH);
  
  for (auto& c : line) {
    if (std::islower(c))
      c = '.';
    else  // std::isupper(c) || c == '_'
      ++num_puyos_;
  }
  pattern_.push_front(line);
}

int Pattern::GetScore(
    const std::map<char, std::map<PuyoColor, int> >& matching) const {
  int sum = 0;
  for (const auto& itr : matching) {
    const std::map<PuyoColor, int>& count = itr.second;
    if (itr.first == '_') {
      auto i = count.find(EMPTY);
      if (i != count.end())
        sum += i->second;
    } else {
      int mx = 0;
      for (const auto& itr2 : count)
        if (itr2.first != EMPTY)
          mx = std::max(mx, itr2.second);
      sum += mx;
    }
  }
  return score_ * sum / num_puyos_;
}

}  // namespace peria
