#ifndef CPU_MAYAH_BOOK_FIELD_H_
#define CPU_MAYAH_BOOK_FIELD_H_

#include <string>
#include <tuple>
#include <vector>

#include "core/plain_field.h"

class BookField {
public:
    struct MatchResult {
        MatchResult(double score, int count) : score(score), count(count) {}

        friend bool operator==(const MatchResult& lhs, const MatchResult& rhs) { return std::tie(lhs.score, lhs.count) == std::tie(rhs.score, rhs.count); }

        double score = 0.0;
        int count = 0;
    };

    BookField(const std::string& name, const std::vector<std::string>& field, double defaultScore = 1);

    // match returns the matched score. If not matched, 0 will be returned.
    MatchResult match(const PlainField&) const;

    void merge(const BookField&);
    BookField mirror() const;

    std::string name() const { return name_; }
    double defaultScore() const { return defaultScore_; }
    double score(int x, int y) const { return scoreField_[x][y]; }

    std::string toDebugString() const;

private:
    std::string name_;
    double defaultScore_;
    char field_[PlainField::MAP_WIDTH][PlainField::MAP_HEIGHT];
    double scoreField_[PlainField::MAP_WIDTH][PlainField::MAP_HEIGHT];
};

#endif
