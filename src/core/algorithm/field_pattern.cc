#include "core/algorithm/field_pattern.h"

#include <glog/logging.h>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <sstream>

#include "core/algorithm/pattern_matcher.h"
#include "core/column_puyo_list.h"
#include "core/core_field.h"
#include "core/field_checker.h"
#include "core/position.h"
#include "core/puyo_color.h"

using namespace std;

namespace {

FieldBits mirror(FieldBits bits) {
    union {
        std::uint16_t xs[8];
        __m128i m;
    };
    m = bits;

    for (int i = 0; i < 4; ++i)
        std::swap(xs[i], xs[7 - i]);

    return m;
}

}

FieldPattern::FieldPattern(const string& field)
{
    int varCount = 0;
    Pattern pats[26];

    int counter = 0;
    for (int i = field.length() - 1; i >= 0; --i) {
        char c = field[i];
        int x = 6 - (counter % 6);
        int y = counter / 6 + 1;
        counter++;

        if (c == '.') {
            continue;
        } else if (c == '*') {
            anyPatternBits_.set(x, y);
        } else if (c == '&') {
            ironPatternBits_.set(x, y);
        } else if ('A' <= c && c <= 'Z') {
            pats[c - 'A'].varBits.set(x, y);
            ++varCount;
        } else if ('a' <= c && c <= 'z') {
            pats[c - 'a'].allowVarBits.set(x, y);
        } else {
            CHECK(false) << "Unacceptable variable " << c << " at (" << x << ", " << y << ")";
        }
    }

    for (int i = 0; i < 26; ++i) {
        pats[i].var = ('A' + i);

        if (pats[i].varBits.isEmpty()) {
            CHECK(pats[i].allowVarBits.isEmpty())
                << "Var " << pats[i].var << " is not used, but allow-var "
                << pats[i].var << " is used.";
            continue;
        }

        patterns_.push_back(pats[i]);
    }

    numVariables_ = varCount;
}

bool FieldPattern::isMatchable(const CoreField& field) const
{
    PatternMatcher matcher;
    return matcher.match(*this, field).matched;
}

bool FieldPattern::isBijectionMatchable() const
{
    if (!anyPatternBits_.isEmpty())
        return false;

    for (const Pattern& pattern : patterns_) {
        if (!pattern.allowVarBits.isEmpty())
            return false;
    }

    return true;
}

FieldBits FieldPattern::patternBits() const
{
    FieldBits bits;
    for (const Pattern& pattern : patterns_) {
        bits.setAll(pattern.varBits);
    }

    return bits;
}

FieldPattern FieldPattern::mirror() const
{
    FieldPattern pf(*this);

    pf.mustPatternBits_ = ::mirror(pf.mustPatternBits_);
    pf.anyPatternBits_ = ::mirror(pf.anyPatternBits_);
    pf.ironPatternBits_ = ::mirror(pf.ironPatternBits_);

    for (Pattern& pat : pf.patterns_) {
        pat.varBits = ::mirror(pat.varBits);
        pat.allowVarBits = ::mirror(pat.allowVarBits);
    }

    return pf;
}

std::string FieldPattern::toDebugString() const
{
    char buf[FieldConstant::MAP_WIDTH][FieldConstant::MAP_HEIGHT] {};
    for (int x = 0; x < FieldConstant::MAP_WIDTH; ++x) {
        for (int y = 0; y < FieldConstant::HEIGHT; ++y) {
            buf[x][y] = ' ';
        }
    }

    anyPatternBits().iterateBitPositions([&buf](int x, int y) {
        buf[x][y] = '*';
    });
    ironPatternBits().iterateBitPositions([&buf](int x, int y) {
        buf[x][y] = '&';
    });
    for (const Pattern& pattern : patterns()) {
        pattern.varBits.iterateBitPositions([&](int x, int y) {
            buf[x][y] = pattern.var;
        });
        pattern.allowVarBits.iterateBitPositions([&](int x, int y) {
            buf[x][y] = std::tolower(pattern.var);
        });
    }

    std::stringstream ss;
    for (int y = 12; y >= 1; --y) {
        for (int x = 1; x <= 6; ++x) {
            ss << buf[x][y];
        }
        ss << endl;
    }
    return ss.str();
}
