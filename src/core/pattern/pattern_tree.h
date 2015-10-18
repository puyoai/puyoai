#ifndef CORE_PATTERN_PATTERN_TREE_H_
#define CORE_PATTERN_PATTERN_TREE_H_

#include <memory>
#include <vector>

#include "core/pattern/pattern_bit.h"

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

class PatternTree {
public:
    PatternTree() {}

    bool isLeaf() const { return patternBookField_.get() != nullptr; }
    const PatternBookField& patternBookField() const { return *patternBookField_; }

    // Returns a node.
    PatternTree* put(const PatternBit&);
    bool setLeaf(const std::string& name, const FieldBits& ironPatternBits, const FieldBits& mustBits,
                 int ignitionColumn, int numVariables, double score);

private:
    friend class PatternBook;

    // If leaf, this is not empty.
    std::unique_ptr<PatternBookField> patternBookField_;
    std::vector<std::pair<PatternBit, std::unique_ptr<PatternTree>>> children_;
};

#endif
