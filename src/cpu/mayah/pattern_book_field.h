#ifndef CPU_MAYAH_PATTERN_BOOK_FIELD_H_
#define CPU_MAYAH_PATTERN_BOOK_FIELD_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <toml/toml.h>

#include "base/noncopyable.h"
#include "base/slice.h"
#include "core/algorithm/field_pattern.h"
#include "core/algorithm/pattern_matcher.h"
#include "core/algorithm/rensa_detector.h"
#include "core/column_puyo_list.h"
#include "core/core_field.h"
#include "core/position.h"

class PatternBookField {
public:
    // If ignitionColumn is 0, we ignore ignitionColumn when detecting rensa.
    PatternBookField(const std::string& field, const std::string& name, int ignitionColumn, double score);

    const std::string& name() const { return name_; }
    double score() const { return score_; }
    int ignitionColumn() const { return ignitionColumn_; }
    const FieldBits& ignitionPositions() const { return ignitionPositions_; }

    const FieldPattern& pattern() const { return pattern_; }
    FieldPattern* mutablePattern() { return &pattern_; }

    int numVariables() const { return pattern_.numVariables(); }

    bool isMatchable(const CoreField& cf) const { return pattern_.isMatchable(cf); }

    ComplementResult complement(const CoreField& cf,
                                int numAllowingFillingUnusedVariables) const
    {
        return PatternMatcher().complement(pattern_, cf, numAllowingFillingUnusedVariables);
    }

    PatternBookField mirror() const
    {
        int mirroredIgnitionColumn = ignitionColumn() == 0 ? 0 : 7 - ignitionColumn();
        return PatternBookField(pattern_.mirror(), name(), mirroredIgnitionColumn, score());
    }

private:
    PatternBookField(const FieldPattern&, const std::string& name, int ignitionColumn, double score);

    FieldPattern pattern_;
    std::string name_;
    int ignitionColumn_;
    double score_;
    FieldBits ignitionPositions_;
};

#endif // CPU_MAYAH_PATTERN_BOOK_H_
