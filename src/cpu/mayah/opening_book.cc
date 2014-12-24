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

class PuyoColorEnv {
public:
    PuyoColorEnv()
    {
        for (int i = 0; i < 26; ++i) {
            map_[i] = PuyoColor::WALL;
        }
    }

    PuyoColor map(char var) const
    {
        DCHECK('A' <= var && var <= 'Z') << var;
        return map_[var - 'A'];
    }

    bool isSet(char var) const
    {
        DCHECK('A' <= var && var <= 'Z') << var;
        return map_[var - 'A'] != PuyoColor::WALL;
    }

    PuyoColor set(char var, PuyoColor pc)
    {
        DCHECK('A' <= var && var <= 'Z') << var;
        return map_[var - 'A'] = pc;
    }

private:
    PuyoColor map_[26];
};

static inline
bool checkCell(char currentVar, char neighborVar, PuyoColor neighborColor, const PuyoColorEnv& env)
{
    DCHECK('A' <= currentVar && currentVar <= 'Z') << currentVar;

    // If neighbor is '*', we don't care what color the cell has.
    if (neighborVar == '*')
        return true;

    if (neighborColor == PuyoColor::OJAMA || neighborColor == PuyoColor::WALL)
        return true;

    // This case should be already processed.
    if (currentVar == neighborVar)
        return true;

    if (neighborVar == '.' || neighborVar == ' ') {
        if (env.map(currentVar) == neighborColor)
            return false;
    } else if ('a' <= neighborVar && neighborVar <= 'z') {
        if (currentVar != std::toupper(neighborVar) && env.map(currentVar) == neighborColor)
            return false;
    } else {
        if (env.map(currentVar) == env.map(neighborVar) && env.isSet(currentVar))
            return false;
    }

    return true;
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

bool OpeningBookField::merge(const OpeningBookField& obf)
{
    return patternField_.merge(obf.patternField_);
}

OpeningBookField::MatchResult OpeningBookField::match(const PlainField& f) const
{
    // First, make a map from char to PuyoColor.
    int matchCount = 0;
    int matchAllowedCount = 0;
    double matchScore = 0;

    // First, create a env (char -> PuyoColor)
    PuyoColorEnv env;
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; f.get(x, y) != PuyoColor::EMPTY; ++y) {
            char c = patternField_.variable(x, y);
            if (c == '.' || c == ' '|| c == '*')
                continue;

            if ('a' <= c && c <= 'z')
                continue;

            DCHECK('A' <= c && c <= 'Z') << c;

            PuyoColor pc = f.get(x, y);
            if (pc == PuyoColor::EMPTY)
                continue;

            if (!isNormalColor(pc))
                return MatchResult(false, 0, 0, 0);

            matchCount += 1;
            matchScore += patternField_.score(x, y);

            if (!env.isSet(c)) {
                env.set(c, pc);
                continue;
            }

            if (env.map(c) != pc)
                return MatchResult(false, 0, 0, 0);
        }
    }

    // Check the neighbors.
    for (int x = 1; x <= 6; ++x) {
        int h = patternField_.height(x);
        for (int y = 1; y <= h; ++y) {
            char c = patternField_.variable(x, y);
            if (c == '*' || c == ' ' || c == '.')
                continue;

            if ('a' <= c && c <= 'z') {
                char uv = std::toupper(patternField_.variable(x, y));
                if (env.isSet(uv) && env.map(uv) == f.get(x, y)) {
                    ++matchAllowedCount;
                }
                continue;
            }

            // Check neighbors.
            if (!checkCell(c, patternField_.variable(x, y + 1), f.get(x, y + 1), env))
                return MatchResult(false, 0, 0, 0);
            if (!checkCell(c, patternField_.variable(x, y - 1), f.get(x, y - 1), env))
                return MatchResult(false, 0, 0, 0);
            if (!checkCell(c, patternField_.variable(x + 1, y), f.get(x + 1, y), env))
                return MatchResult(false, 0, 0, 0);
            if (!checkCell(c, patternField_.variable(x - 1, y), f.get(x - 1, y), env))
                return MatchResult(false, 0, 0, 0);
        }
    }

    return MatchResult(true, matchScore, matchCount, matchAllowedCount);
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
        OpeningBookField field(current);
        if (!field.merge(it->second))
            continue;
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

        partialFields.emplace(name, OpeningBookField(name, field, score));
    }

    const toml::Array& combines = value.find("combine")->as<toml::Array>();
    for (const auto& combine : combines) {
        string combinedName = combine.get<string>("name");
        vector<string> names;
        for (const auto& s : combine.get<toml::Array>("combine")) {
            names.push_back(s.as<string>());
        }

        auto range = partialFields.equal_range(names[0]);
        for (auto it = range.first; it != range.second; ++it) {
            merge(&fields_, it->second, partialFields, names, 1);
        }
    }

    return true;
}
