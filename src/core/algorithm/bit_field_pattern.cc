#include "bit_field_pattern.h"

using namespace std;

BitFieldPattern::BitFieldPattern(const string& field, double score) :
    score_(score)
{
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
}
