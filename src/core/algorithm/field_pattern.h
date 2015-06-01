#ifndef CORE_ALGORITHM_PATTERN_FIELD_H_
#define CORE_ALGORITHM_PATTERN_FIELD_H_

#include <string>
#include <vector>

#include "core/field_bits.h"
#include "core/field_constant.h"

class CoreField;

class FieldPattern {
public:
    struct Pattern {
        char var;
        FieldBits varBits;
        FieldBits allowVarBits;
    };

    explicit FieldPattern(const std::string&);

    bool isBijectionMatchable() const;
    // 'A' - 'Z' is 1, the others are 0.
    FieldBits patternBits() const;

    bool isMatchable(const CoreField&) const;

    int numVariables() const { return numVariables_; }
    void setMustVar(int x, int y) { mustPatternBits_.set(x, y); }

    const std::vector<Pattern>& patterns() const { return patterns_; }
    const Pattern& pattern(size_t idx) const { return patterns_[idx]; }

    const FieldBits& mustPatternBits() const { return mustPatternBits_; }
    const FieldBits& anyPatternBits() const { return anyPatternBits_; }
    const FieldBits& ironPatternBits() const { return ironPatternBits_; }

    FieldPattern mirror() const;
    std::string toDebugString() const;

private:
    int numVariables_;
    FieldBits mustPatternBits_;
    FieldBits anyPatternBits_;
    FieldBits ironPatternBits_;
    std::vector<Pattern> patterns_;
};

#endif // CORE_ALGORITHM_PATTERN_FIELD_H_
