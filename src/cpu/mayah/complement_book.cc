#include "complement_book.h"

#include <fstream>
#include <glog/logging.h>

#include "core/algorithm/pattern_matcher.h"

using namespace std;

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
