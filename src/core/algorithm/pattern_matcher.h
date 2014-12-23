#ifndef CORE_ALGORITHM_PATTERN_MATCHER_H_
#define CORE_ALGORITHM_PATTERN_MATCHER_H_

#include "core/puyo_color.h"

class BijectionMatcher {
public:
    BijectionMatcher();

    bool match(char, PuyoColor);

private:
    PuyoColor colors_[4];
    char chars_[NUM_PUYO_COLORS];
};

#endif
