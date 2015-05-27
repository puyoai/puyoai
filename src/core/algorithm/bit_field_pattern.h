#ifndef CORE_ALGORITHM_BIT_FIELD_PATTERN_H_
#define CORE_ALGORITHM_BIT_FIELD_PATTERN_H_

#include <string>
#include <vector>

#include "core/field_bits.h"

class BitFieldPattern {
public:
    struct Pattern {
        char var;
        FieldBits varBits;
        FieldBits allowVarBits;
    };

    explicit BitFieldPattern(const std::string&, double score = 1.0);

    double score() const { return score_; }

    const std::vector<Pattern>& patterns() const { return patterns_; }
    const Pattern& pattern(size_t idx) const { return patterns_[idx]; }

    const FieldBits& anyPatternBits() const { return anyPatternBits_; }
    const FieldBits& ironPatternBits() const { return ironPatternBits_; }

private:

    double score_;
    FieldBits anyPatternBits_;
    FieldBits ironPatternBits_;
    std::vector<Pattern> patterns_;
};

#endif // CORE_ALGORITHM_BIT_FIELD_PATTERN_H_
