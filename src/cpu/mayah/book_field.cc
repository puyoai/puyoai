#include "book_field.h"

#include <algorithm>
#include <iostream>
#include <map>
#include "core/puyo_color.h"

using namespace std;

static inline
bool check(char currentVar, char neighborVar, PuyoColor neighborColor,
           const map<char, PuyoColor>& env)
{
    DCHECK(currentVar != '.');

    if (!isNormalColor(neighborColor))
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

BookField::BookField(const string& name, const vector<string>& field, double score, bool partial) :
    name_(name),
    score_(score),
    partial_(partial)
{
    for (int x = 0; x < PlainField::MAP_WIDTH; ++x) {
        for (int y = 0; y < PlainField::MAP_HEIGHT; ++y) {
            field_[x][y] = '.';
        }
    }

    for (size_t i = 0; i < field.size(); ++i) {
        CHECK_EQ(field[i].size(), 6U);
        int y = static_cast<int>(field.size()) - i;
        for (int x = 1; x <= 6; ++x) {
            char var = field[i][x - 1];
            if (var != '.')
                ++varCount_;
            field_[x][y] = var;
        }
    }
}

void BookField::merge(const BookField& bf)
{
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            if (field_[x][y] == '.' && bf.field_[x][y] != '.') {
                field_[x][y] = bf.field_[x][y];
                ++varCount_;
            }
        }
    }
}

BookField BookField::mirror() const
{
    BookField bf(*this);
    for (int x = 1; x <= 3; ++x) {
        for (int y = 1; y <= 12; ++y) {
            swap(bf.field_[x][y], bf.field_[7-x][y]);
        }
    }

    return bf;
}

int BookField::matchCount(const PlainField& f) const
{
    // First, make a map from char to PuyoColor.
    int matchedCount = 0;
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
                return 0;

            ++matchedCount;

            if (!env.count(c)) {
                env[c] = pc;
                continue;
            }

            if (env[c] != pc)
                return 0;
        }
    }

    if (matchedCount == 0)
        return 0;

    // Check the neighbors.
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12 && (f.get(x, y) != PuyoColor::EMPTY || field_[x][y] != '.'); ++y) {
            if (field_[x][y] == '.')
                continue;

            if (!check(field_[x][y], field_[x][y + 1], f.get(x, y + 1), env))
                return 0;
            if (!check(field_[x][y], field_[x + 1][y], f.get(x + 1, y), env))
                return 0;
            if (!check(field_[x][y], field_[x - 1][y], f.get(x - 1, y), env))
                return 0;
        }
    }

    return matchedCount;
}
