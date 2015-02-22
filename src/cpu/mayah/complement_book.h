#ifndef CPU_MAYAH_COMPLEMENT_BOOK_H_
#define CPU_MAYAH_COMPLEMENT_BOOK_H_

#include <string>
#include <toml/toml.h>

#include "base/noncopyable.h"
#include "core/algorithm/column_puyo_list.h"
#include "core/algorithm/pattern_field.h"
#include "core/core_field.h"

class ColumnPuyoList;

class ComplementBookField {
public:
    explicit ComplementBookField(const PatternField& field) : patternField_(field), ignoreables{} {}
    explicit ComplementBookField(const std::string& field) : patternField_(field), ignoreables{} {}
    explicit ComplementBookField(const std::vector<std::string>& field) : patternField_(field), ignoreables{} {}

    bool isMatchable(const CoreField&) const;
    bool complement(const CoreField&, ColumnPuyoList*) const;

    // If ignoreable is set, complement() allows the corresponding PuyoColor of the ignoreable char is PuyoColor::EMPTY.
    bool isIgnoreable(char c) const;
    void setIgnoreable(char c);

    ComplementBookField mirror() const { return ComplementBookField(patternField_.mirror()); }

private:
    PatternField patternField_;
    bool ignoreables[26];
};

class ComplementBook : noncopyable {
public:
    bool load(const std::string& filename);
    bool loadFromValue(const toml::Value&);

    size_t size() const { return fields_.size(); }
    const ComplementBookField& field(int i) const { return fields_[i]; }

private:
    std::vector<ComplementBookField> fields_;
};

#endif
