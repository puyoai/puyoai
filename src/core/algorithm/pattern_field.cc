#include "pattern_field.h"

#include <algorithm>

#include <glog/logging.h>

using namespace std;

PatternField::PatternField() :
    heights_{}
{
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            field_[x][y] = ' ';
        }
    }
}

PatternField::PatternField(const std::string& field) :
    PatternField()
{
    int counter = 0;
    for (int i = field.length() - 1; i >= 0; --i) {
        int c = field[i];
        int x = 6 - (counter % 6);
        int y = counter / 6 + 1;

        if (c != '.' && c != ' ') {
            field_[x][y] = c;
            heights_[x] = std::max(height(x), y);
        }
        counter++;
    }
}

PatternField::PatternField(const vector<string>& field) :
    PatternField()
{
    for (size_t i = 0; i < field.size(); ++i) {
        CHECK_EQ(field[i].size(), 6U);

        int y = static_cast<int>(field.size()) - i;
        for (int x = 1; x <= 6; ++x) {
            if (field[i][x - 1] == '.')
                continue;

            field_[x][y] = field[i][x - 1];
            heights_[x] = std::max(height(x), y);
        }
    }
}
