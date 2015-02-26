#include "complement_book.h"

#include <fstream>
#include <glog/logging.h>

#include "core/algorithm/pattern_matcher.h"

using namespace std;

bool ComplementBookField::isMatchable(const CoreField& field) const
{
    PatternMatcher matcher;
    return matcher.match(patternField_, field).matched;
}

bool ComplementBookField::complement(const CoreField& field, ColumnPuyoList* cpl) const
{
    PatternMatcher matcher;
    if (!matcher.match(patternField_, field).matched)
        return false;

    int currentHeights[FieldConstant::MAP_WIDTH] {
        0, field.height(1), field.height(2), field.height(3),
        field.height(4), field.height(5), field.height(6), 0
    };

    cpl->clear();
    for (int x = 1; x <= 6; ++x) {
        int h = patternField_.height(x);
        for (int y = 1; y <= h; ++y) {
            if (patternField_.type(x, y) != PatternType::VAR &&
                patternField_.type(x, y) != PatternType::MUST_VAR) {
                if (field.color(x, y) == PuyoColor::EMPTY)
                    return false;
                continue;
            }
            char c = patternField_.variable(x, y);
            if (!matcher.isSet(c))
                return false;
            if (ColumnPuyoList::MAX_SIZE <= cpl->size())
                return false;
            if (patternField_.type(x, y) == PatternType::MUST_VAR) {
                if (!isNormalColor(field.color(x, y)))
                    return false;
            }
            if (field.color(x, y) == PuyoColor::EMPTY && currentHeights[x] + 1 == y) {
                cpl->add(x, matcher.map(c));
                currentHeights[x]++;
            }
        }
    }

    return true;
}

bool ComplementBook::load(const std::string& filename)
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

bool ComplementBook::loadFromValue(const toml::Value& patterns)
{
    const toml::Array& vs = patterns.find("pattern")->as<toml::Array>();
    fields_.reserve(vs.size());
    for (const toml::Value& v : vs) {
        vector<string> f;
        for (const auto& s : v.get<toml::Array>("field"))
            f.push_back(s.as<string>());
        ComplementBookField patternBookField(f);
        fields_.push_back(patternBookField);
        fields_.push_back(patternBookField.mirror());
    }

    return true;
}
