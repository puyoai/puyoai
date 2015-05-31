#include "core/algorithm/field_pattern.h"

#include <glog/logging.h>

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <sstream>

#include "core/algorithm/pattern_matcher.h"
#include "core/column_puyo_list.h"
#include "core/core_field.h"
#include "core/field_checker.h"
#include "core/position.h"
#include "core/puyo_color.h"

using namespace std;

FieldPattern::FieldPattern() :
    heights_{},
    numVariables_(0)
{
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y) {
            vars_[x][y] = ' ';
            types_[x][y] = PatternType::NONE;
        }
    }

    for (int y = 0; y < MAP_HEIGHT; ++y) {
        vars_[0][y] = '\0';
        types_[0][y] = PatternType::WALL;
        vars_[MAP_WIDTH - 1][y] = '\0';
        types_[MAP_WIDTH - 1][y] = PatternType::WALL;
    }

    for (int x = 0; x < MAP_WIDTH; ++x) {
        vars_[x][0] = '\0';
        types_[x][0] = PatternType::WALL;
        vars_[x][MAP_HEIGHT - 1] = '\0';
        types_[x][MAP_HEIGHT - 1] = PatternType::WALL;
    }
}

FieldPattern::FieldPattern(const std::string& field) :
    FieldPattern()
{
    int counter = 0;
    for (int i = field.length() - 1; i >= 0; --i) {
        char c = field[i];
        int x = 6 - (counter % 6);
        int y = counter / 6 + 1;
        counter++;

        PatternType t = inferType(c);
        setPattern(x, y, t, c);
    }

    numVariables_ = countVariables();
}

FieldPattern::FieldPattern(const vector<string>& field) :
    FieldPattern()
{
    for (size_t i = 0; i < field.size(); ++i) {
        CHECK_EQ(field[i].size(), 6U);

        int y = static_cast<int>(field.size()) - i;
        for (int x = 1; x <= 6; ++x) {
            char c = field[i][x - 1];
            PatternType t = inferType(c);
            setPattern(x, y, t, c);
        }
    }

    numVariables_ = countVariables();
}

bool FieldPattern::isMatchable(const CoreField& field) const
{
    PatternMatcher matcher;
    return matcher.match(*this, field).matched;
}

void FieldPattern::setPattern(int x, int y, PatternType t, char variable)
{
    switch (t) {
    case PatternType::NONE:
        break;
    case PatternType::ANY:
        types_[x][y] = t;
        vars_[x][y] = '*';
        heights_[x] = std::max(height(x), y);
        break;
    case PatternType::VAR:
    case PatternType::MUST_VAR:
        CHECK('A' <= variable && variable <= 'Z');
        types_[x][y] = t;
        vars_[x][y] = variable;
        heights_[x] = std::max(height(x), y);
        break;
    case PatternType::ALLOW_VAR:
        CHECK(('A' <= variable && variable <= 'Z') || ('a' <= variable && variable <= 'z'));
        types_[x][y] = t;
        vars_[x][y] = std::toupper(variable);
        heights_[x] = std::max(height(x), y);
        break;
    case PatternType::ALLOW_FILLING_IRON:
        CHECK_EQ(variable, '&');
        types_[x][y] = t;
        vars_[x][y] = '&';
        heights_[x] = std::max(height(x), y);
        break;
    default:
        CHECK(false);
    }
}

// static
PatternType FieldPattern::inferType(char c)
{
    if (c == ' ' || c == '.')
        return PatternType::NONE;
    if (c == '*')
        return PatternType::ANY;
    if (c == '&')
        return PatternType::ALLOW_FILLING_IRON;
    if ('A' <= c && c <= 'Z')
        return PatternType::VAR;
    if ('a' <= c && c <= 'z')
        return PatternType::ALLOW_VAR;

    return PatternType::NONE;
}

Position* FieldPattern::fillSameVariablePositions(int x, int y, char c, Position* positionQueueHead, FieldChecker* checked) const
{
    DCHECK(!checked->get(x, y));

    if (FieldConstant::HEIGHT < y)
        return positionQueueHead;

    Position* writeHead = positionQueueHead;
    Position* readHead = positionQueueHead;

    *writeHead++ = Position(x, y);
    checked->set(x, y);

    while (readHead != writeHead) {
        Position p = *readHead++;

        if (variable(p.x + 1, p.y) == c && !checked->get(p.x + 1, p.y)) {
            *writeHead++ = Position(p.x + 1, p.y);
            checked->set(p.x + 1, p.y);
        }
        if (variable(p.x - 1, p.y) == c && !checked->get(p.x - 1, p.y)) {
            *writeHead++ = Position(p.x - 1, p.y);
            checked->set(p.x - 1, p.y);
        }
        if (variable(p.x, p.y + 1) == c && !checked->get(p.x, p.y + 1) && p.y + 1 <= FieldConstant::HEIGHT) {
            *writeHead++ = Position(p.x, p.y + 1);
            checked->set(p.x, p.y + 1);
        }
        if (variable(p.x, p.y - 1) == c && !checked->get(p.x, p.y - 1)) {
            *writeHead++ = Position(p.x, p.y - 1);
            checked->set(p.x, p.y - 1);
        }
    }

    return writeHead;
}

FieldPattern FieldPattern::mirror() const
{
    FieldPattern pf(*this);
    for (int x = 1; x <= 3; ++x) {
        std::swap(pf.heights_[x], pf.heights_[7 - x]);
        for (int y = 1; y < MAP_HEIGHT; ++y) {
            std::swap(pf.vars_[x][y], pf.vars_[7 - x][y]);
            std::swap(pf.types_[x][y], pf.types_[7 - x][y]);
        }
    }

    return pf;
}

std::string FieldPattern::toDebugString() const
{
    std::stringstream ss;
    for (int y = 12; y >= 1; --y) {
        for (int x = 1; x <= 6; ++x) {
            ss << variable(x, y);
        }
        ss << endl;
    }
    return ss.str();
}

int FieldPattern::countVariables() const
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
