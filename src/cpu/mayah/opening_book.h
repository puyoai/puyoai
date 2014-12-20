#ifndef CPU_MAYAH_OPENING_BOOK_H_
#define CPU_MAYAH_OPENING_BOOK_H_

#include <string>
#include <tuple>
#include <vector>

#include "base/noncopyable.h"
#include "core/field_constant.h"
class PlainField;

class OpeningBookField : FieldConstant {
public:
    struct MatchResult {
        MatchResult(bool matched, double score, int count, int allowedCount) :
            matched(matched), score(score), count(count), allowedCount(allowedCount) {}

        friend bool operator==(const MatchResult& lhs, const MatchResult& rhs)
        {
            return std::tie(lhs.matched, lhs.score, lhs.count, lhs.allowedCount) == std::tie(rhs.matched, rhs.score, rhs.count, rhs.allowedCount);
        }

        bool matched;
        double score;
        int count;
        int allowedCount;
    };

    OpeningBookField(const std::string& name, const std::vector<std::string>& field, double defaultScore = 1);

    // match returns the matched score. If not matched, 0 will be returned.
    MatchResult match(const PlainField&) const;

    bool merge(const OpeningBookField&);
    OpeningBookField mirror() const;

    std::string name() const { return name_; }
    double defaultScore() const { return defaultScore_; }
    double score(int x, int y) const { return scoreField_[x][y]; }

    int varCount() const { return varCount_; }

    std::string toDebugString() const;

private:
    int calculateVarCount() const;

    std::string name_;
    double defaultScore_;
    int varCount_;
    char field_[MAP_WIDTH][MAP_HEIGHT];
    double scoreField_[MAP_WIDTH][MAP_HEIGHT];
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
