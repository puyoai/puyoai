#ifndef CORE_ALGORITHM_BIJECTION_MATCHER_H_
#define CORE_ALGORITHM_BIJECTION_MATCHER_H_

#include "core/puyo_color.h"

class FieldPattern;
class CoreField;

// BijectionMatcher is a pattern matcher s.t.
//  - each variables should match different PuyoColor.
//  - pattern variable should be 'A'-'D'.
class BijectionMatcher {
public:
    BijectionMatcher();

    bool match(const FieldPattern&, const CoreField&);
    bool match(char, PuyoColor);

private:
    PuyoColor colors_[4];
    char chars_[NUM_PUYO_COLORS];
};

#endif
