#ifndef CPU_MAYAH_PATTERN_BOOK_H_
#define CPU_MAYAH_PATTERN_BOOK_H_

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
#include "core/pattern/pattern_matcher.h"
#include "core/position.h"

#include "pattern_book_field.h"

class PatternBook : noncopyable {
public:
    typedef std::unordered_map<FieldBits, std::vector<int>> IndexMap;
    typedef std::vector<int>::const_iterator IndexIterator;

    bool load(const std::string& filename);
    bool loadFromString(const std::string&);
    bool loadFromValue(const toml::Value&);

    // Finds the PatternBookField from the positions where puyos are erased at the first chain.
    // Multiple PatternBookField might be found, so begin-iterator and end-iterator will be
    // returned. If no such PatternBookField is found, begin-iterator and end-iterator are the same.
    std::pair<IndexIterator, IndexIterator> find(FieldBits ignitionPositions) const;

    size_t size() const { return fields_.size(); }
    const PatternBookField& patternBookField(int i) const { return fields_[i]; }

private:
    std::vector<PatternBookField> fields_;
    IndexMap index_;
    std::vector<FieldBits> indexKeys_;
};

class NewPatternBookField {
public:
    NewPatternBookField(const std::string& name, const FieldBits& ironBits,
                        int ignitionColumn, int numVariables, double score) :
        name_(name), ironBits_(ironBits), ignitionColumn_(ignitionColumn), numVariables_(numVariables), score_(score) {}

    const std::string name() const { return name_; }
    const FieldBits& ironBits() const { return ironBits_; }
    int ignitionColumn() const { return ignitionColumn_; }
    int numVariables() const { return numVariables_; }
    double score() const { return score_; }

private:
    std::string name_;
    FieldBits ironBits_;
    int ignitionColumn_;
    int numVariables_;
    double score_;
};

class NewPatternBook : noncopyable {
public:
    typedef std::function<void (CoreField&& complementedField,
                                const ColumnPuyoList& complementedPuyoList,
                                int numFilledUnusedVariables,
                                const FieldBits& matchedBits,
                                const NewPatternBookField&)> ComplementCallback;

    NewPatternBook();
    ~NewPatternBook();

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
