#include "pattern_field.h"

#include <algorithm>

#include <glog/logging.h>

using namespace std;

PatternField::PatternField(double defaultScore) :
    heights_{},
    numVariables_(0)
{
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            vars_[x][y] = ' ';
            scores_[x][y] = defaultScore;
        }
    }
}

PatternField::PatternField(const std::string& field, double defaultScore) :
    PatternField(defaultScore)
{
    int counter = 0;
    for (int i = field.length() - 1; i >= 0; --i) {
        int c = field[i];
        int x = 6 - (counter % 6);
        int y = counter / 6 + 1;

        if (c != '.' && c != ' ') {
            vars_[x][y] = c;
            heights_[x] = std::max(height(x), y);
        }
        counter++;
    }

    numVariables_ = countVariables();
}

PatternField::PatternField(const vector<string>& field, double defaultScore) :
    PatternField(defaultScore)
{
    for (size_t i = 0; i < field.size(); ++i) {
        CHECK_EQ(field[i].size(), 6U);

        int y = static_cast<int>(field.size()) - i;
        for (int x = 1; x <= 6; ++x) {
            if (field[i][x - 1] == '.' || field[i][x - 1] == ' ')
                continue;

            vars_[x][y] = field[i][x - 1];
            heights_[x] = std::max(height(x), y);
        }
    }

    numVariables_ = countVariables();
}

PatternType PatternField::type(int x, int y) const
{
    char c = variable(x, y);
    if (c == ' ' || c == '.')
        return PatternType::EMPTY;
    if (c == '*')
        return PatternType::ANY;
    if ('A' <= c && c <= 'Z')
        return PatternType::MUST_VAR;
    if ('a' <= c && c <= 'z')
        return PatternType::ALLOW_VAR;

    return PatternType::MUST;
}

PatternField PatternField::mirror() const
{
    PatternField pf(*this);
    for (int x = 1; x < 3; ++x) {
        std::swap(pf.heights_[x], pf.heights_[7 - x]);
        for (int y = 1; y < MAP_HEIGHT; ++y) {
            std::swap(pf.vars_[x][y], pf.vars_[7 - x][y]);
            std::swap(pf.scores_[x][y], pf.scores_[7 - x][y]);
        }
    }

    return pf;
}

bool PatternField::merge(const PatternField& pf)
{
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            if (pf.variable(x, y) == '.' || pf.variable(x, y) == ' ')
                continue;
            if (variable(x, y) == '.' || variable(x, y) == ' ') {
                setVariable(x, y, pf.variable(x, y));
                setScore(x, y, pf.score(x, y));
                heights_[x] = std::max(height(x), y);
                continue;
            }

            if (variable(x, y) != '*' && pf.variable(x, y) == '*')
                continue;
            if (variable(x, y) == '*' && pf.variable(x, y) != '*') {
                setVariable(x, y, pf.variable(x, y));
                setScore(x, y, pf.score(x, y));
                heights_[x] = std::max(height(x), y);
                continue;
            }

            if (('A' <= variable(x, y) && variable(x, y) <= 'Z') &&
                ('a' <= pf.variable(x, y) && pf.variable(x, y) <= 'z')) {
                continue;
            } else if (('a' <= variable(x, y) && variable(x, y) <= 'z') &&
                       ('A' <= pf.variable(x, y) && pf.variable(x, y) <= 'Z')) {
                setVariable(x, y, pf.variable(x, y));
                setScore(x, y, pf.score(x, y));
                heights_[x] = std::max(height(x), y);
                continue;
            }

            setScore(x, y, std::max(score(x, y), pf.score(x, y)));

            if (variable(x, y) != pf.variable(x, y)) {
                VLOG(1) << "These field cannot be merged: "
                        << toDebugString() << '\n'
                        << pf.toDebugString();
                return false;
            }
        }
    }

    numVariables_ = countVariables();
    return true;
}

std::string PatternField::toDebugString() const
{
    stringstream ss;
    for (int y = 12; y >= 1; --y) {
        for (int x = 1; x <= 6; ++x) {
            ss << variable(x, y);
        }
        ss << endl;
    }
    return ss.str();
}

int PatternField::countVariables() const
{
    int count = 0;
    for (int x = 1; x <= WIDTH; ++x) {
        for (int y = 1; y <= HEIGHT; ++y) {
            if ('A' <= variable(x, y) && variable(x, y) <= 'Z') {
                ++count;
            }
        }
    }

    return count;
}
