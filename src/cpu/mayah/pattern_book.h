#ifndef CPU_MAYAH_PATTERN_BOOK_H_
#define CPU_MAYAH_PATTERN_BOOK_H_

#include <string>
#include <toml/toml.h>

#include "base/noncopyable.h"
#include "core/algorithm/column_puyo_list.h"
#include "core/algorithm/pattern_field.h"
#include "core/core_field.h"

class ColumnPuyoList;

class PatternBookField {
public:
    explicit PatternBookField(const PatternField& field) : patternField_(field), ignoreables{} {}
    explicit PatternBookField(const std::string& field) : patternField_(field), ignoreables{} {}
    explicit PatternBookField(const std::vector<std::string>& field) : patternField_(field), ignoreables{} {}

    bool isMatchable(const CoreField&) const;
    bool complement(const CoreField&, ColumnPuyoList*) const;

    // If ignoreable is set, complement() allows the corresponding PuyoColor of the ignoreable char is PuyoColor::EMPTY.
    bool isIgnoreable(char c) const;
    void setIgnoreable(char c);

    PatternBookField mirror() const { return PatternBookField(patternField_.mirror()); }

private:
    PatternField patternField_;
    char ignoreables[26];
};

class PatternBook : noncopyable {
public:
    bool load(const std::string& filename);
    bool loadFromValue(const toml::Value&);

    size_t size() const { return fields_.size(); }
    const PatternBookField& field(int i) const { return fields_[i]; }

private:
    std::vector<PatternBookField> fields_;
};

#endif
