#include "pattern_book.h"

#include <algorithm>
#include <fstream>

using namespace std;

bool PatternBook::load(const string& filename)
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

bool PatternBook::loadFromString(const string& str)
{
    istringstream ss(str);
    toml::Parser parser(ss);
    toml::Value v = parser.parse();
    if (!v.valid()) {
        LOG(ERROR) << parser.errorReason();
        return false;
    }

    return loadFromValue(v);
}

bool PatternBook::loadFromValue(const toml::Value& patterns)
{
    CHECK(fields_.empty());
    CHECK(index_.empty());

    const toml::Array& vs = patterns.find("pattern")->as<toml::Array>();
    for (const toml::Value& v : vs) {
        string str;
        for (const auto& s : v.get<toml::Array>("field"))
            str += s.as<string>();

        string name;
        if (const toml::Value* p = v.find("name")) {
            name = p->as<string>();
        }

        int ignitionColumn = 0;
        if (const toml::Value* p = v.find("ignition")) {
            ignitionColumn = p->as<int>();
            CHECK(1 <= ignitionColumn && ignitionColumn <= 6) << ignitionColumn;
        }

        double score = 0;
        if (const toml::Value* p = v.find("score")) {
            if (p->is<int>())
                score = p->as<int>();
            else if (p->is<double>())
                score = p->as<double>();
            else
                CHECK(false);
        }

        PatternBookField pbf(str, name, ignitionColumn, score);

        if (const toml::Value* p = v.find("precondition")) {
            for (const auto& cp : p->as<toml::Array>()) {
                int x = cp.get<int>(0);
                int y = cp.get<int>(1);
                pbf.mutablePattern()->setMustVar(x, y);
            }
        }

        fields_.push_back(pbf);
        fields_.push_back(pbf.mirror());
    }

    for (int i = 0; i < static_cast<int>(fields_.size()); ++i) {
        FieldBits ignitionPositions = fields_[i].ignitionPositions();
        auto it = index_.find(ignitionPositions);
        if (it != index_.end()) {
            it->second.push_back(i);
            continue;
        }

        // No such key.
        index_.emplace(ignitionPositions, vector<int>{i});
        indexKeys_.push_back(ignitionPositions);
    }

    return true;
}

pair<PatternBook::IndexIterator, PatternBook::IndexIterator>
PatternBook::find(FieldBits ignitionPositions) const
{
    auto it = index_.find(ignitionPositions);
    if (it != index_.end())
        return make_pair(it->second.begin(), it->second.end());

    static const vector<int> s_emptyVector;
    return make_pair(s_emptyVector.begin(), s_emptyVector.end());
}
