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

#endif // CPU_MAYAH_PATTERN_BOOK_H_
