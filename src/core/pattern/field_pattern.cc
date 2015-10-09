#include "core/pattern/field_pattern.h"

#include <glog/logging.h>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <sstream>

#include "core/column_puyo_list.h"
#include "core/core_field.h"
#include "core/field_checker.h"
#include "core/position.h"
#include "core/puyo_color.h"

using namespace std;

FieldPattern::FieldPattern(const string& field, const string& notPatternField)
{
    int varCount = 0;
    struct TempPattern {
        FieldBits varBits;
        FieldBits allowVarBits;
        FieldBits notVarBits;
    } pats[26];
    FieldBits emptyBits;

    int counter = 0;
    for (int i = field.length() - 1; i >= 0; --i) {
        char c = field[i];
        int x = 6 - (counter % 6);
        int y = counter / 6 + 1;
        counter++;

        if (c == '.') {
            emptyBits.set(x, y);
            continue;
        } else if (c == '*') {
            anyPatternBits_.set(x, y);
        } else if (c == '&') {
            ironPatternBits_.set(x, y);
        } else if ('A' <= c && c <= 'Z') {
            pats[c - 'A'].varBits.set(x, y);
            ++varCount;
        } else if ('a' <= c && c <= 'z') {
            emptyBits.set(x, y);
            pats[c - 'a'].allowVarBits.set(x, y);
        } else {
            CHECK(false) << "Unacceptable variable " << c << " at (" << x << ", " << y << ")";
        }
    }

    counter = 0;
    for (int i = notPatternField.length() - 1; i >= 0; --i) {
        char c = notPatternField[i];
        int x = 6 - (counter % 6);
        int y = counter / 6 + 1;
        counter++;

        if (c == '.') {
            continue;
        }

        if ('A' <= c && c <= 'Z') {
            pats[c - 'A'].notVarBits.set(x, y);
            continue;
        }

        CHECK(false) << "Unacceptable variable " << c << " at (" << x << ", " << y << ")";
    }

    for (int i = 0; i < 26; ++i) {
        Pattern p;

        p.var = static_cast<char>('A' + i);
        p.varBits = pats[i].varBits;

        if (pats[i].varBits.isEmpty()) {
            CHECK(pats[i].allowVarBits.isEmpty())
                << "Var " << p.var << " is not used, but allow-var "
                << p.var << " is used.";
            CHECK(pats[i].notVarBits.isEmpty())
                << "Var " << p.var << " is not used, but not-var "
                << p.var << " is used.";
            continue;
        }

        p.notVarBits = p.varBits.expandEdge().notmask(p.varBits).notmask(pats[i].allowVarBits).notmask(anyPatternBits_) | (pats[i].notVarBits);
        p.notVarBits = p.notVarBits.maskedField13();
        patterns_.push_back(p);
    }

    numVariables_ = varCount;
}

bool FieldPattern::isBijectionMatchable() const
{
    if (!anyPatternBits_.isEmpty())
        return false;

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

    pf.mustPatternBits_ = pf.mustPatternBits_.mirror();
    pf.anyPatternBits_ = pf.anyPatternBits_.mirror();
    pf.ironPatternBits_ = pf.ironPatternBits_.mirror();

    for (Pattern& pat : pf.patterns_) {
        pat.varBits = pat.varBits.mirror();
        pat.notVarBits = pat.notVarBits.mirror();
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
#if 0
        pattern.notVarBits.iterateBitPositions([&](int x, int y) {
            buf[x][y] = std::tolower(pattern.var);
        });
#endif
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
