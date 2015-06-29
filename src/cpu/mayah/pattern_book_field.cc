#include "pattern_book_field.h"

#include <algorithm>
#include <fstream>

using namespace std;

namespace {

FieldBits findIgnitionPositions(const FieldPattern& pattern)
{
    Position tmp[128];

    for (const auto& pat : pattern.patterns()) {
        int len = pat.varBits.toPositions(tmp);
        for (int i = 0; i < len; ++i) {
            FieldBits x = FieldBits(tmp[i].x, tmp[i].y).expand(pat.varBits);
            if (x.popcount() >= 4)
                return x;
        }
    }

    CHECK(false) << "there is no 4-connected variables." << pattern.toDebugString();
    return FieldBits();
}

} // anonymous namespace

PatternBookField::PatternBookField(const string& name, const string& field, const string& notField, int ignitionColumn, double score) :
    name_(name),
    pattern_(field, notField),
    ignitionColumn_(ignitionColumn),
    score_(score),
    ignitionPositions_(findIgnitionPositions(pattern_))
{
    DCHECK(0 <= ignitionColumn && ignitionColumn <= 6);
}

PatternBookField::PatternBookField(const string& name, const FieldPattern& pattern, int ignitionColumn, double score) :
    name_(name),
    pattern_(pattern),
    ignitionColumn_(ignitionColumn),
    score_(score),
    ignitionPositions_(findIgnitionPositions(pattern_))
{
    DCHECK(0 <= ignitionColumn && ignitionColumn <= 6);
}
