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
#include "core/pattern/pattern_tree.h"
#include "core/position.h"

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
    bool loadFromString(const std::string&, bool ignoreDuplicate = false);
    bool loadFromValue(const toml::Value&, bool ignoreDuplicate = false);

    void complement(const CoreField&, const ComplementCallback&) const;
    void complement(const CoreField&, int allowedNumUnusedVariables, const ComplementCallback&) const;
    void complement(const CoreField&, const FieldBits& ignitionBits, int allowedNumUnusedVariables, const ComplementCallback&) const;

private:
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
