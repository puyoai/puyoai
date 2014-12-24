#ifndef CPU_MAYAH_OPENING_BOOK_H_
#define CPU_MAYAH_OPENING_BOOK_H_

#include <string>
#include <tuple>
#include <vector>

#include "base/noncopyable.h"
#include "core/field_constant.h"
#include "core/algorithm/pattern_field.h"
#include "core/algorithm/pattern_matcher.h"

class OpeningBookField : FieldConstant {
public:
    OpeningBookField(const std::string& name, const std::vector<std::string>& field, double defaultScore = 1);
    OpeningBookField(const std::string& name, const PatternField&);

    // match returns the matched score. If not matched, 0 will be returned.
    PatternMatchResult match(const CoreField&) const;

    bool merge(const OpeningBookField&);
    OpeningBookField mirror() const { return OpeningBookField(name(), patternField_.mirror()); }

    std::string name() const { return name_; }
    std::string toDebugString() const;

private:
    std::string name_;
    PatternField patternField_;
};

class OpeningBook : noncopyable {
public:
    bool load(const std::string& filename);

    std::string toString() const;

    size_t size() const { return fields_.size(); }
    const std::vector<OpeningBookField>& fields() const { return fields_; }
    const OpeningBookField& field(int ith) const { return fields_[ith]; }

private:
    std::vector<OpeningBookField> fields_;
};

#endif
