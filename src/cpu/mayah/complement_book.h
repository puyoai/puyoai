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
    explicit ComplementBookField(const PatternField& field) : patternField_(field) {}
    explicit ComplementBookField(const std::string& field) : patternField_(field) {}
    explicit ComplementBookField(const std::vector<std::string>& field) : patternField_(field) {}

    bool isMatchable(const CoreField&) const;
    bool complement(const CoreField&, ColumnPuyoList*) const;

    ComplementBookField mirror() const { return ComplementBookField(patternField_.mirror()); }

private:
    PatternField patternField_;
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
