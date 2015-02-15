#include "opening_book.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#include <toml/toml.h>

#include "core/puyo_color.h"
#include "core/plain_field.h"

using namespace std;

OpeningBookField::OpeningBookField(const string& name) :
    name_(name)
{
}

OpeningBookField::OpeningBookField(const string& name, const vector<string>& field, double defaultScore) :
    name_(name),
    patternField_(field, defaultScore)
{
}

OpeningBookField::OpeningBookField(const string& name, const PatternField& patternField) :
    name_(name),
    patternField_(patternField)
{
}

PatternMatchResult OpeningBookField::match(const CoreField& cf) const
{
    PatternMatcher matcher;
    return matcher.match(patternField_, cf);
}

bool OpeningBookField::preMatch(const CoreField& cf) const
{
    PatternMatcher matcher;
    return matcher.match(patternField_, cf, true).matched;
}

string OpeningBookField::toDebugString() const
{
    return patternField_.toDebugString();
}

string OpeningBook::toString() const
{
    stringstream ss;
    for (const auto& bf : fields_) {
        ss << bf.toDebugString() << '\n';
    }
    return ss.str();
}

static void merge(vector<OpeningBookField>* result,
                  const OpeningBookField& current,
                  const multimap<string, OpeningBookField>& partialFields,
                  const vector<string>& names,
                  size_t pos)
{
    if (pos == names.size()) {
        result->push_back(current);
        result->push_back(current.mirror());
        return;
    }

    auto range = partialFields.equal_range(names[pos]);
    for (auto it = range.first; it != range.second; ++it) {
        OpeningBookField field(current.name());
        if (!PatternField::merge(current.patternField(), it->second.patternField(), field.mutablePatternField())) {
            LOG(INFO) << "couldn't merge: \n" << current.patternField().toDebugString() << '\n' << it->second.patternField().toDebugString();
            continue;
        }
        merge(result, field, partialFields, names, pos + 1);
    }
}

bool OpeningBook::load(const string& filename)
{
    multimap<string, OpeningBookField> partialFields;

    ifstream ifs(filename);
    toml::Value value;
    try {
        toml::Parser parser(ifs);
        value = parser.parse();
        if (!value.valid()) {
            LOG(ERROR) << parser.errorReason();
            return false;
        }
    } catch (std::exception& e) {
        LOG(ERROR) << e.what();
        return false;
    }

    const toml::Array& books = value.find("book")->as<toml::Array>();
    for (const auto& book : books) {
        string name = book.get<string>("name");
        double score = book.get<double>("score");
        vector<string> field;
        for (const auto& s : book.get<toml::Array>("field"))
            field.push_back(s.as<string>());

        OpeningBookField obf(name, field, score);
        if (const toml::Value* precond = book.find("precondition")) {
            for (const auto& v : precond->as<toml::Array>()) {
                int x = v.get<int>(0);
                int y = v.get<int>(1);
                PatternField* pf = obf.mutablePatternField();
                CHECK(pf->type(x, y) == PatternType::VAR);
                pf->setType(x, y, PatternType::MUST_VAR);
            }
        }

        partialFields.emplace(name, obf);
    }

    const toml::Array& combines = value.find("combine")->as<toml::Array>();
    for (const auto& combine : combines) {
        string combinedName = combine.get<string>("name");
        vector<string> names;
        for (const auto& s : combine.get<toml::Array>("combine")) {
            names.push_back(s.as<string>());
        }

        OpeningBookField obf(combinedName);
        merge(&fields_, obf, partialFields, names, 0);
    }

    return true;
}
