#include "pattern_book.h"

#include <algorithm>
#include <fstream>

#include "core/field_bit_field.h"

using namespace std;

namespace {

vector<Position> findIgnitionPositions(const FieldPattern& pattern)
{
    Position positions[FieldConstant::MAP_WIDTH * FieldConstant::MAP_HEIGHT];

    FieldBitField checked;
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            if (checked(x, y))
                continue;
            if (!(pattern.type(x, y) == PatternType::VAR || pattern.type(x, y) == PatternType::MUST_VAR))
                continue;
            char c = pattern.variable(x, y);
            CHECK('A' <= c && c <= 'Z');
            Position* p = pattern.fillSameVariablePositions(x, y, c, positions, &checked);
            if (p - positions >= 4) {
                std::sort(positions, p);
                return vector<Position>(positions, p);
            }
        }
    }

    CHECK(false) << "there is no 4-connected variables." << pattern.toDebugString();
    return vector<Position>();
}

} // anonymous namespace

const vector<int> PatternBook::s_emptyVector;

PatternBookField::PatternBookField(const string& field, const string& name, int ignitionColumn, double score) :
    pattern_(field, score),
    name_(name),
    ignitionColumn_(ignitionColumn),
    score_(score),
    ignitionPositions_(findIgnitionPositions(pattern_))
{
    DCHECK(0 <= ignitionColumn && ignitionColumn <= 6);
}

PatternBookField::PatternBookField(const FieldPattern& pattern, const string& name, int ignitionColumn, double score) :
    pattern_(pattern),
    name_(name),
    ignitionColumn_(ignitionColumn),
    score_(score),
    ignitionPositions_(findIgnitionPositions(pattern_))
{
    DCHECK(0 <= ignitionColumn && ignitionColumn <= 6);
}

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
                CHECK(pbf.pattern().type(x, y) == PatternType::VAR);
                pbf.mutablePattern()->setType(x, y, PatternType::MUST_VAR);
            }
        }

        fields_.push_back(pbf);
        fields_.push_back(pbf.mirror());
    }

    index_.clear();
    for (int i = 0; i < static_cast<int>(fields_.size()); ++i) {
        index_[fields_[i].ignitionPositions()].push_back(i);
    }

    return true;
}

pair<PatternBook::IndexIterator, PatternBook::IndexIterator>
PatternBook::find(const vector<Position>& ignitionPositions) const
{
    auto it = index_.find(ignitionPositions);
    if (it != index_.end())
        return make_pair(it->second.begin(), it->second.end());

    return make_pair(s_emptyVector.begin(), s_emptyVector.end());
}
