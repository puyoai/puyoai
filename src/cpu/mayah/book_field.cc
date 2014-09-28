#include "book_field.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include "core/puyo_color.h"

using namespace std;

static inline
bool check(char currentVar, char neighborVar, PuyoColor neighborColor,
           const map<char, PuyoColor>& env)
{
    DCHECK(currentVar != '.');

    if (neighborColor == PuyoColor::OJAMA || neighborColor == PuyoColor::WALL)
        return true;

    // This case should be already processed.
    if (currentVar == neighborVar)
        return true;

    if (neighborVar == '.') {
        auto it = env.find(currentVar);
        if (it != env.end() && it->second == neighborColor)
            return false;
    } else {
        auto it = env.find(currentVar);
        auto jt = env.find(neighborVar);

        if (it != env.end() && jt != env.end() && it->second == jt->second)
            return false;
    }

    return true;
}

BookField::BookField(const string& name, const vector<string>& field, double defaultScore) :
    name_(name),
    defaultScore_(defaultScore)
{
    for (int x = 0; x < PlainField::MAP_WIDTH; ++x) {
        for (int y = 0; y < PlainField::MAP_HEIGHT; ++y) {
            field_[x][y] = '.';
            scoreField_[x][y] = 0;
        }
    }

    for (size_t i = 0; i < field.size(); ++i) {
        CHECK_EQ(field[i].size(), 6U);
        int y = static_cast<int>(field.size()) - i;
        for (int x = 1; x <= 6; ++x) {
           CHECK(('A' <= field[i][x - 1] && field[i][x - 1] <= 'Z') ||
                 ('a' <= field[i][x - 1] && field[i][x - 1] <= 'z') ||
                 (field[i][x - 1] == '.'));

            if ('A' <= field[i][x - 1] && field[i][x - 1] <= 'Z') {
                field_[x][y] = field[i][x - 1];
                scoreField_[x][y] = defaultScore;
            }
        }
    }

    varCount_ = calculateVarCount();
}

void BookField::merge(const BookField& bf)
{
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            if (bf.field_[x][y] == '.')
                continue;

            if (field_[x][y] == '.') {
                field_[x][y] = bf.field_[x][y];
                scoreField_[x][y] = bf.scoreField_[x][y];
            } else {
                CHECK_EQ(field_[x][y], bf.field_[x][y]) << name() << " : " << bf.name()
                                                        << "x=" << x << " y=" << y;
                scoreField_[x][y] = bf.scoreField_[x][y];
            }
        }
    }

    varCount_ = calculateVarCount();
}

BookField BookField::mirror() const
{
    BookField bf(*this);
    for (int x = 1; x <= 3; ++x) {
        for (int y = 1; y <= 12; ++y) {
            swap(bf.field_[x][y], bf.field_[7-x][y]);
            swap(bf.scoreField_[x][y], bf.scoreField_[7-x][y]);
        }
    }

    return bf;
}

BookField::MatchResult BookField::match(const PlainField& f) const
{
    // First, make a map from char to PuyoColor.
    int matchCount = 0;
    double matchScore = 0;
    map<char, PuyoColor> env;
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; f.get(x, y) != PuyoColor::EMPTY; ++y) {
            char c = field_[x][y];
            if (c == '.')
                continue;

            PuyoColor pc = f.get(x, y);
            if (pc == PuyoColor::EMPTY)
                continue;

            if (!isNormalColor(pc))
                return MatchResult(0, 0);

            matchCount += 1;
            matchScore += scoreField_[x][y];

            if (!env.count(c)) {
                env[c] = pc;
                continue;
            }

            if (env[c] != pc)
                return MatchResult(0, 0);
        }
    }

    if (matchScore == 0)
        return MatchResult(0, 0);

    // Check the neighbors.
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12 && field_[x][y] != '.'; ++y) {
            if (!check(field_[x][y], field_[x][y + 1], f.get(x, y + 1), env))
                return MatchResult(0, 0);
            if (!check(field_[x][y], field_[x][y - 1], f.get(x, y - 1), env))
                return MatchResult(0, 0);
            if (!check(field_[x][y], field_[x + 1][y], f.get(x + 1, y), env))
                return MatchResult(0, 0);
            if (!check(field_[x][y], field_[x - 1][y], f.get(x - 1, y), env))
                return MatchResult(0, 0);
        }
    }

    return MatchResult(matchScore, matchCount);
}

string BookField::toDebugString() const
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

int BookField::calculateVarCount() const
{
    int count = 0;
    for (int x = 1; x <= FieldConstant::WIDTH; ++x) {
        for (int y = 1; y <= FieldConstant::HEIGHT; ++y) {
            if ('A' <= field_[x][y] && field_[x][y] <= 'Z') {
                ++count;
            }
        }
    }

    return count;
}
