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
    DCHECK_NE(currentVar, '.');
    DCHECK_NE(currentVar, '*');
    DCHECK('A' <= currentVar && currentVar <= 'Z') << currentVar;

    // If neighbor is '*', we don't care what color the cell has.
    if (neighborVar == '*')
        return true;

    if (neighborColor == PuyoColor::OJAMA || neighborColor == PuyoColor::WALL)
        return true;

    // This case should be already processed.
    if (currentVar == neighborVar)
        return true;

    if (neighborVar == '.') {
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
    defaultScore_(defaultScore)
{
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            field_[x][y] = '.';
            scoreField_[x][y] = 0;
        }
    }

    for (size_t i = 0; i < field.size(); ++i) {
        CHECK_EQ(field[i].size(), 6U);
        int y = static_cast<int>(field.size()) - i;
        for (int x = 1; x <= 6; ++x) {
            if (field[i][x - 1] == '.')
                continue;

            if ('A' <= field[i][x - 1] && field[i][x - 1] <= 'Z') {
                field_[x][y] = field[i][x - 1];
                scoreField_[x][y] = defaultScore;
                continue;
            }

            if ('a' <= field[i][x - 1] && field[i][x - 1] <= 'z') {
                field_[x][y] = field[i][x - 1];
                continue;
            }

            if (field[i][x - 1] == '*') {
                field_[x][y] = field[i][x - 1];
                continue;
            }

            CHECK(false) << "Unknown field character: " << field[i][x - 1];
        }
    }

    varCount_ = calculateVarCount();
}

bool OpeningBookField::merge(const OpeningBookField& obf)
{
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            if (obf.field_[x][y] == '.') {
                continue;
            } else if (field_[x][y] == '.') {
                field_[x][y] = obf.field_[x][y];
                scoreField_[x][y] = obf.scoreField_[x][y];
                continue;
            }

            if (field_[x][y] != '*' && obf.field_[x][y] == '*') {
                continue;
            } else if (field_[x][y] == '*' && obf.field_[x][y] != '*') {
                field_[x][y] = obf.field_[x][y];
                scoreField_[x][y] = obf.scoreField_[x][y];
                continue;
            }

            if (('A' <= field_[x][y] && field_[x][y] <= 'Z') &&
                ('a' <= obf.field_[x][y] && obf.field_[x][y] <= 'z')) {
            } else if (('a' <= field_[x][y] && field_[x][y] <= 'z') &&
                       ('A' <= obf.field_[x][y] && obf.field_[x][y] <= 'Z')) {
                field_[x][y] = obf.field_[x][y];
            } else if (field_[x][y] != obf.field_[x][y]) {
                VLOG(1) << "These field cannot be merged: "
                        << toDebugString() << '\n'
                        << obf.toDebugString();
                return false;
            }
            scoreField_[x][y] = std::max(scoreField_[x][y], obf.scoreField_[x][y]);
        }
    }

    varCount_ = calculateVarCount();
    return true;
}

OpeningBookField OpeningBookField::mirror() const
{
    OpeningBookField obf(*this);
    for (int x = 1; x <= 3; ++x) {
        for (int y = 1; y <= 12; ++y) {
            swap(obf.field_[x][y], obf.field_[7 - x][y]);
            swap(obf.scoreField_[x][y], obf.scoreField_[7 - x][y]);
        }
    }

    return obf;
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
            char c = field_[x][y];
            if (c == '.' || c == '*')
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
            matchScore += scoreField_[x][y];

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
        for (int y = 1; y <= 12 && field_[x][y] != '.'; ++y) {
            if (field_[x][y] == '*')
                continue;

            if ('a' <= field_[x][y] && field_[x][y] <= 'z') {
                char c = std::toupper(field_[x][y]);
                if (env.isSet(c) && env.map(c) == f.get(x, y)) {
                    ++matchAllowedCount;
                }
                continue;
            }

            // Check neighbors.
            if (!checkCell(field_[x][y], field_[x][y + 1], f.get(x, y + 1), env))
                return MatchResult(false, 0, 0, 0);
            if (!checkCell(field_[x][y], field_[x][y - 1], f.get(x, y - 1), env))
                return MatchResult(false, 0, 0, 0);
            if (!checkCell(field_[x][y], field_[x + 1][y], f.get(x + 1, y), env))
                return MatchResult(false, 0, 0, 0);
            if (!checkCell(field_[x][y], field_[x - 1][y], f.get(x - 1, y), env))
                return MatchResult(false, 0, 0, 0);
        }
    }

    return MatchResult(true, matchScore, matchCount, matchAllowedCount);
}

string OpeningBookField::toDebugString() const
{
    stringstream ss;
    for (int y = 12; y >= 1; --y) {
        for (int x = 1; x <= 6; ++x) {
            ss << field_[x][y];
        }
        ss << endl;
    }
    return ss.str();
}

int OpeningBookField::calculateVarCount() const
{
    int count = 0;
    for (int x = 1; x <= WIDTH; ++x) {
        for (int y = 1; y <= HEIGHT; ++y) {
            if ('A' <= field_[x][y] && field_[x][y] <= 'Z') {
                ++count;
            }
        }
    }

    return count;
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
