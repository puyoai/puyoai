#ifndef CORE_PATTERN_PATTERN_BOOK_H_
#define CORE_PATTERN_PATTERN_BOOK_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <toml/toml.h>

#include "base/noncopyable.h"
#include "core/rensa/rensa_detector.h"
#include "core/column_puyo_list.h"
#include "core/core_field.h"
#include "core/pattern/field_pattern.h"
#include "core/pattern/pattern_bit.h"
#include "core/position.h"

class PatternBookField {
public:
    PatternBookField(const std::string& name, const FieldBits& ironBits, const FieldBits& mustBits,
                        int ignitionColumn, int numVariables, double score) :
        name_(name), ironBits_(ironBits), mustBits_(mustBits),
        ignitionColumn_(ignitionColumn), numVariables_(numVariables), score_(score) {}

    const std::string name() const { return name_; }
    const FieldBits& ironBits() const { return ironBits_; }
    const FieldBits& mustBits() const { return mustBits_; }
    int ignitionColumn() const { return ignitionColumn_; }
    int numVariables() const { return numVariables_; }
    double score() const { return score_; }

private:
    std::string name_;
    FieldBits ironBits_;
    FieldBits mustBits_;
    int ignitionColumn_;
    int numVariables_;
    double score_;
};

class PatternBook : noncopyable {
public:
    typedef std::function<void (CoreField&& complementedField,
                                const ColumnPuyoList& complementedPuyoList,
                                int numFilledUnusedVariables,
                                const FieldBits& matchedBits,
                                const PatternBookField&)> ComplementCallback;

    PatternBook();
    ~PatternBook();

    bool load(const std::string& filename);
    bool loadFromString(const std::string&);
    bool loadFromValue(const toml::Value&);

    void complement(const CoreField&, const ComplementCallback&) const;
    void complement(const CoreField&, int allowedNumUnusedVariables, const ComplementCallback&) const;
    void complement(const CoreField&, const FieldBits& ignitionBits, int allowedNumUnusedVariables, const ComplementCallback&) const;

private:
    class PatternTree;
    void iterate(const PatternTree&,
                 const CoreField& oridinalField,
                 const BitField& currentField,
                 const FieldBits& matchedBits,
                 int allowedNumUnusedVariables,
                 int numUnusedVariables,
                 const ComplementCallback&) const;

    std::unique_ptr<PatternTree> root_;
};

#endif // CPU_MAYAH_PATTERN_BOOK_H_
