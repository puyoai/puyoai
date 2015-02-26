#ifndef CPU_MAYAH_COMPLEMENT_BOOK_H_
#define CPU_MAYAH_COMPLEMENT_BOOK_H_

#include <string>
#include <toml/toml.h>

#include "base/noncopyable.h"
#include "core/algorithm/column_puyo_list.h"
#include "core/algorithm/field_pattern.h"
#include "core/core_field.h"

class ColumnPuyoList;

class ComplementBookField {
public:
    explicit ComplementBookField(const FieldPattern& field) : pattern_(field) {}
    explicit ComplementBookField(const std::string& field) : pattern_(field) {}
    explicit ComplementBookField(const std::vector<std::string>& field) : pattern_(field) {}

    bool isMatchable(const CoreField& cf) const { return pattern_.isMatchable(cf); }
    bool complement(const CoreField& cf, ColumnPuyoList* cpl) const { return pattern_.complement(cf, cpl); }

    ComplementBookField mirror() const { return ComplementBookField(pattern_.mirror()); }

private:
    FieldPattern pattern_;
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
