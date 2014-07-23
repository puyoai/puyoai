#include "pattern.h"

namespace peria {

namespace {

std::vector<Pattern> g_pattern;

}  // namespace

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
