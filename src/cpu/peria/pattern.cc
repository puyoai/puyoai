#include "pattern.h"

#include <cctype>
#include <deque>
#include <istream>
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
      pattern->set_score(score);
    } else if (first_segment == "MAX:") {
      int max_puyos;
      iss >> max_puyos;
      pattern->set_max_puyos(max_puyos);
    }
  }

  if (pattern)
    g_pattern.push_back(*pattern);
}

const std::vector<Pattern>& Pattern::GetAllPattern() {
  return g_pattern;
}

int Pattern::Match(const CoreField& field) const {
  UNUSED_VARIABLE(field);
  // TODO: Check also 左右反転 version.
  return 0;
}

void Pattern::AppendField(std::string line) {
  for (auto& c : line) {
    if (std::islower(c))
      c = '.';
  }
  pattern_.push_front(line);
}

}  // namespace peria
