#ifndef CORE_ALGORITHM_BIT_PATTERN_MATCHER_H_
#define CORE_ALGORITHM_BIT_PATTERN_MATCHER_H_

#include "core/algorithm/pattern_match_result.h"

class BitFieldPattern;
class BitField;

class BitPatternMatcher {
public:
    bool match(const BitFieldPattern&, const BitField&);
};

#endif // CORE_ALGORITHM_BIT_PATTERN_MATCHER_H_
