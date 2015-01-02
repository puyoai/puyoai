#include "pattern_book.h"

#include <fstream>
#include <glog/logging.h>

#include "core/algorithm/pattern_matcher.h"

using namespace std;

bool PatternBookField::isMatchable(const CoreField& field) const
{
    PatternMatcher matcher;
    return matcher.match(patternField_, field).matched;
}

bool PatternBookField::complement(const CoreField& field, ColumnPuyoList* cpl) const
{
    PatternMatcher matcher;
    if (!matcher.match(patternField_, field).matched)
        return false;

    cpl->clear();
    for (int x = 1; x <= 6; ++x) {
        int h = patternField_.height(x);
        for (int y = 1; y <= h; ++y) {
            if (patternField_.type(x, y) != PatternType::MUST_VAR) {
                if (field.color(x, y) == PuyoColor::EMPTY)
                    return false;
                continue;
            }
            char c = patternField_.variable(x, y);
            if (!matcher.isSet(c))
                return false;
            if (ColumnPuyoList::MAX_SIZE <= cpl->size())
                return false;
            if (field.color(x, y) == PuyoColor::EMPTY)
                cpl->add(x, matcher.map(c));
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
        PatternBookField patternBookField(f);
        fields_.push_back(patternBookField);
        fields_.push_back(patternBookField.mirror());
    }

    return true;
}
