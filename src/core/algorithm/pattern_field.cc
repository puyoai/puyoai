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
            types_[x][y] = PatternType::NONE;
        }
    }
}

PatternField::PatternField(const std::string& field, double defaultScore) :
    PatternField(defaultScore)
{
    int counter = 0;
    for (int i = field.length() - 1; i >= 0; --i) {
        char c = field[i];
        int x = 6 - (counter % 6);
        int y = counter / 6 + 1;
        counter++;

        PatternType t = inferType(c);
        setPattern(x, y, t, c, defaultScore);
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
            char c = field[i][x - 1];
            PatternType t = inferType(c);
            setPattern(x, y, t, c, defaultScore);
        }
    }

    numVariables_ = countVariables();
}

// static
bool PatternField::merge(const PatternField& pf1, const PatternField& pf2, PatternField* pf)
{
/*
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {

            PatternType pt1 = type(x, y);
            PatternType pt2 = pf.type(x, y);

            if (pt2 == PatternType::NONE)
                continue;
            if (pt1 == PatternType::NONE) {
                set(x, y, pf.type(x, y), pf.variable(x, y), pf.score(x, y));
                continue;
            }

            if (pt1 != PatternType::ANY && pt2 == PatternType::ANY)
                continue;
            if (pt1 == PatternType::ANY && pt2 != PatternType::ANY) {
                set(x, y, pf.type(x, y), pf.variable(x, y), pf.score(x, y));
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
*/
    return false;
}

void PatternField::setPattern(int x, int y, PatternType t, char variable, double score)
{
    switch (t) {
    case PatternType::NONE:
        break;
    case PatternType::ANY:
        types_[x][y] = t;
        vars_[x][y] = '*';
        scores_[x][y] = score;
        heights_[x] = std::max(height(x), y);
        break;
    case PatternType::MUST_EMPTY:
        types_[x][y] = t;
        vars_[x][y] = '_';
        scores_[x][y] = score;
        heights_[x] = std::max(height(x), y);
        break;
    case PatternType::MUST_VAR:
        CHECK('A' <= variable && variable <= 'Z');
        types_[x][y] = t;
        vars_[x][y] = variable;
        scores_[x][y] = score;
        heights_[x] = std::max(height(x), y);
        break;
    case PatternType::ALLOW_VAR:
        CHECK(('A' <= variable && variable <= 'Z') || ('a' <= variable && variable <= 'z'));
        types_[x][y] = t;
        vars_[x][y] = std::toupper(variable);
        scores_[x][y] = score;
        heights_[x] = std::max(height(x), y);
        break;
    case PatternType::NOT_VAR:
        CHECK(('A' <= variable && variable <= 'Z') || ('a' <= variable && variable <= 'z'));
        types_[x][y] = t;
        vars_[x][y] = std::toupper(variable);
        scores_[x][y] = score;
        heights_[x] = std::max(height(x), y);
        break;
    default:
        CHECK(false);
    }
}

// static
PatternType PatternField::inferType(char c, PatternType typeForLowerCase)
{
    if (c == ' ' || c == '.')
        return PatternType::NONE;
    if (c == '*')
        return PatternType::ANY;
    if (c == '_')
        return PatternType::MUST_EMPTY;
    if ('A' <= c && c <= 'Z')
        return PatternType::MUST_VAR;
    if ('a' <= c && c <= 'z')
        return typeForLowerCase;

    return PatternType::NONE;
}

PatternField PatternField::mirror() const
{
    PatternField pf(*this);
    for (int x = 1; x <= 3; ++x) {
        std::swap(pf.heights_[x], pf.heights_[7 - x]);
        for (int y = 1; y < MAP_HEIGHT; ++y) {
            std::swap(pf.vars_[x][y], pf.vars_[7 - x][y]);
            std::swap(pf.types_[x][y], pf.types_[7 - x][y]);
            std::swap(pf.scores_[x][y], pf.scores_[7 - x][y]);
        }
    }

    return pf;
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
            if ('A' <= variable(x, y) && variable(x, y) <= 'Z')
                ++count;
        }
    }

    return count;
}
