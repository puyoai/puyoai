#include "pattern_book.h"

#include <fstream>
#include <glog/logging.h>

using namespace std;

bool PatternBookField::complement(const CoreField& field, ColumnPuyoList* cpl)
{
    PuyoColor cs[10] {
        PuyoColor::EMPTY, PuyoColor::EMPTY, PuyoColor::EMPTY, PuyoColor::EMPTY, PuyoColor::EMPTY,
        PuyoColor::EMPTY, PuyoColor::EMPTY, PuyoColor::EMPTY, PuyoColor::EMPTY, PuyoColor::EMPTY,
    };

    int maxIndex = 0;
    for (int x = 1; x <= 6; ++x) {
        int h = patternField_.height(x);
        for (int y = 1; y <= h; ++y) {
            char c = patternField_.variable(x, y);
            if (c == ' ') {
                if (field.color(x, y) == PuyoColor::EMPTY)
                    return false;
                continue;
            }
            if (c == '*') {
                if (field.color(x, y) != PuyoColor::EMPTY)
                    return false;
                continue;
            }

            if (c < '1' || '9' < c)
                continue;
            int i = c - '0';
            maxIndex = std::max(i, maxIndex);
            if (cs[i] == PuyoColor::EMPTY) {
                if (field.color(x, y) == PuyoColor::OJAMA)
                    return false;
                if (isNormalColor(field.color(x, y)))
                    cs[i] = field.color(x, y);
            } else {
                if (cs[i] != field.color(x, y) && field.color(x, y) != PuyoColor::EMPTY)
                    return false;
            }
        }
    }

    // If some variable is not matched, the template should be considered as unmatchched.
    for (int i = 1; i <= maxIndex; ++i) {
        if (cs[i] == PuyoColor::EMPTY)
            return false;
    }

    cpl->clear();
    for (int x = 1; x <= 6; ++x) {
        int h = patternField_.height(x);
        for (int y = 1; y <= h; ++y) {
            if (patternField_.variable(x, y) == ' ')
                continue;
            if (field.color(x, y) != PuyoColor::EMPTY)
                continue;
            if (ColumnPuyoList::MAX_SIZE <= cpl->size())
                return false;
            int i = patternField_.variable(x, y) == '*' ? 1 : patternField_.variable(x, y) - '0';
            cpl->addPuyo(x, cs[i]);
        }
    }

    return true;
}

bool PatternBook::load(const std::string& filename)
{
    ifstream ifs(filename);
    toml::Parser parser(ifs);
    toml::Value v = parser.parse();
    if (!v.valid()) {
        LOG(ERROR) << parser.errorReason();
        return false;
    }

    return loadFromValue(std::move(v));
}

bool PatternBook::loadFromValue(const toml::Value& patterns)
{
    const toml::Array& vs = patterns.find("pattern")->as<toml::Array>();
    fields_.reserve(vs.size());
    for (const toml::Value& v : vs) {
        vector<string> f;
        for (const auto& s : v.get<toml::Array>("field"))
            f.push_back(s.as<string>());
        fields_.emplace_back(f);
    }

    return true;
}
