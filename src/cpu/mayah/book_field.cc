#include "book_field.h"

#include <iostream>
#include <map>
#include "core/puyo_color.h"

using namespace std;

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
        for (int y = 1; f.get(x, y) != PuyoColor::EMPTY; ++y) {
            if (field_[x][y] == '.')
                continue;

            // up
            if (isNormalColor(f.get(x, y + 1))) {
                if (field_[x][y + 1] == '.') {
                    if (f.get(x, y) == f.get(x, y + 1))
                        return false;
                } else {
                    if (field_[x][y] == field_[x][y + 1])
                        continue;
                    if (f.get(x, y) == f.get(x, y + 1))
                        return false;
                }
            }

            // right
            if (f.get(x + 1, y) != PuyoColor::EMPTY) {
                if (field_[x + 1][y] == '.') {
                    if (f.get(x + 1, y) == f.get(x, y))
                        return false;
                } else {
                    if (field_[x][y] == field_[x + 1][y])
                        continue;
                    if (f.get(x, y) == f.get(x + 1, y))
                        return false;
                }
            }

            // right
            if (f.get(x - 1, y) != PuyoColor::EMPTY) {
                if (field_[x - 1][y] == '.') {
                    if (f.get(x - 1, y) == f.get(x, y))
                        return false;
                } else {
                    if (field_[x][y] == field_[x - 1][y])
                        continue;
                    if (f.get(x, y) == f.get(x - 1, y))
                        return false;
                }
            }
        }
    }


    return true;
}
