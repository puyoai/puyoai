#include "pattern.h"

#include <istream>
#include <string>

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
  for (std::string line; std::getline(is, line);) {
    line = Trim(line);
    if (line.empty())
      continue;
  }
}

const std::vector<Pattern>& Pattern::GetAllPattern() {
  return g_pattern;
}

Pattern::Pattern(const std::string& pattern) : pattern_(pattern) {
}

int Pattern::Match(const CoreField& field) const {
  UNUSED_VARIABLE(field);
  return 0;
}

}  // namespace peria
