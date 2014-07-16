#include "book_field.h"

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

BookField::BookField(const string& name, const vector<string>& field, bool partial) :
    name_(name),
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
            field_[x][y] = field[i][x - 1];
        }
    }
}

void BookField::merge(const BookField& bf)
{
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            if (field_[x][y] == '.')
                field_[x][y] = bf.field_[x][y];
        }
    }
}

bool BookField::matches(const PlainField& f) const
{
    // First, make a map from char to PuyoColor.
    map<char, PuyoColor> env;
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; f.get(x, y) != PuyoColor::EMPTY; ++y) {
            PuyoColor pc = f.get(x, y);
            if (pc == PuyoColor::EMPTY)
                continue;

            char c = field_[x][y];
            if (c == '.')
                continue;

            if (!isNormalColor(pc))
                return false;

            if (!env.count(c)) {
                env[c] = pc;
                continue;
            }

            if (env[c] != pc)
                return false;
        }
    }

    // Check the neighbors (Only up and right.)
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; f.get(x, y) != PuyoColor::EMPTY || field_[x][y] != '.'; ++y) {
            if (field_[x][y] == '.')
                continue;

            if (!check(field_[x][y], field_[x][y + 1], f.get(x, y + 1), env))
                return false;
            if (!check(field_[x][y], field_[x + 1][y], f.get(x + 1, y), env))
                return false;
            if (!check(field_[x][y], field_[x - 1][y], f.get(x - 1, y), env))
                return false;
        }
    }


    return true;
}
