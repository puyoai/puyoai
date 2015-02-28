#include "core/algorithm/field_pattern.h"

#include <algorithm>
#include <sstream>

#include <glog/logging.h>

#include "core/algorithm/pattern_matcher.h"

using namespace std;

FieldPattern::FieldPattern(double defaultScore) :
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

FieldPattern::FieldPattern(const std::string& field, double defaultScore) :
    FieldPattern(defaultScore)
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

FieldPattern::FieldPattern(const vector<string>& field, double defaultScore) :
    FieldPattern(defaultScore)
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
bool FieldPattern::merge(const FieldPattern& pf1, const FieldPattern& pf2, FieldPattern* pf)
{
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            PatternType pt1 = pf1.type(x, y);
            PatternType pt2 = pf2.type(x, y);
            if (pt1 == PatternType::NONE) {
                pf->setPattern(x, y, pf2.type(x, y), pf2.variable(x, y), pf2.score(x, y));
                continue;
            } else if (pt2 == PatternType::NONE) {
                pf->setPattern(x, y, pf1.type(x, y), pf1.variable(x, y), pf1.score(x, y));
                continue;
            }

            if (pt1 == PatternType::ANY) {
                if (pt2 == PatternType::MUST_EMPTY)
                    return false;
                pf->setPattern(x, y, pf2.type(x, y), pf2.variable(x, y), pf2.score(x, y));
                continue;
            }
            if (pt2 == PatternType::ANY) {
                if (pt1 == PatternType::MUST_EMPTY)
                    return false;
                pf->setPattern(x, y, pf1.type(x, y), pf1.variable(x, y), pf1.score(x, y));
                continue;
            }

            if (pt1 == PatternType::MUST_EMPTY) {
                if (pt2 != PatternType::MUST_EMPTY)
                    return false;
                pf->setPattern(x, y, pf1.type(x, y), pf1.variable(x, y), pf1.score(x, y));
                continue;
            }
            if (pt2 == PatternType::MUST_EMPTY) {
                if (pt1 != PatternType::MUST_EMPTY)
                    return false;
                pf->setPattern(x, y, pf2.type(x, y), pf2.variable(x, y), pf2.score(x, y));
                continue;
            }

            if (pt1 == PatternType::MUST_VAR && pt2 == PatternType::MUST_VAR) {
                if (pf1.variable(x, y) != pf2.variable(x, y))
                    return false;
            }
            if (pt1 == PatternType::MUST_VAR) {
                if (pt2 != PatternType::VAR)
                    return false;
                if (pf1.variable(x, y) != pf2.variable(x, y))
                    return false;
                pf->setPattern(x, y, pt1, pf1.variable(x, y), std::max(pf1.score(x, y), pf2.score(x, y)));
                continue;
            }
            if (pt2 == PatternType::MUST_VAR) {
                if (pt1 != PatternType::VAR)
                    return false;
                if (pf2.variable(x, y) != pf1.variable(x, y))
                    return false;
                pf->setPattern(x, y, pt2, pf2.variable(x, y), std::max(pf1.score(x, y), pf2.score(x, y)));
                continue;
            }

            if (pt1 == PatternType::VAR && pt2 == PatternType::VAR) {
                if (pf1.variable(x, y) != pf2.variable(x, y))
                    return false;
                pf->setPattern(x, y, pf1.type(x, y), pf1.variable(x, y), std::max(pf1.score(x, y), pf2.score(x, y)));
                continue;
            }
            if (pt1 == PatternType::VAR) {
                if (pt2 == PatternType::ALLOW_VAR || pt2 == PatternType::NOT_VAR) {
                    pf->setPattern(x, y, pf1.type(x, y), pf1.variable(x, y), pf1.score(x, y));
                    continue;
                }
                return false;
            }
            if (pt2 == PatternType::VAR) {
                if (pt1 == PatternType::ALLOW_VAR || pt1 == PatternType::NOT_VAR) {
                    pf->setPattern(x, y, pf2.type(x, y), pf2.variable(x, y), pf2.score(x, y));
                    continue;
                }
                return false;
            }

            return false;
        }
    }

    pf->numVariables_ = pf->countVariables();
    return true;
}

bool FieldPattern::isMatchable(const CoreField& field) const
{
    PatternMatcher matcher;
    return matcher.match(*this, field).matched;
}

bool FieldPattern::complement(const CoreField& field, bool allowsFillOjama, ColumnPuyoList* cpl) const
{
    PatternMatcher matcher;
    if (!matcher.match(*this, field).matched)
        return false;

    int currentHeights[FieldConstant::MAP_WIDTH] {
        0, field.height(1), field.height(2), field.height(3),
        field.height(4), field.height(5), field.height(6), 0
    };

    cpl->clear();
    for (int x = 1; x <= 6; ++x) {
        int h = height(x);
        for (int y = 1; y <= h; ++y) {
            if (allowsFillOjama && type(x, y) == PatternType::NONE && field.color(x, y) == PuyoColor::EMPTY) {
                if (!cpl->add(x, PuyoColor::OJAMA))
                    return false;
                ++currentHeights[x];
                continue;
            }
            if (!(type(x, y) == PatternType::VAR || type(x, y) == PatternType::MUST_VAR)) {
                if (field.color(x, y) == PuyoColor::EMPTY) {
                    return false;
                }
                continue;
            }
            char c = variable(x, y);
            if (!matcher.isSet(c))
                return false;
            if (ColumnPuyoList::MAX_SIZE <= cpl->size())
                return false;
            if (type(x, y) == PatternType::MUST_VAR) {
                if (!isNormalColor(field.color(x, y)))
                    return false;
            }
            if (field.color(x, y) == PuyoColor::EMPTY && currentHeights[x] + 1 == y) {
                if (!cpl->add(x, matcher.map(c)))
                    return false;
                ++currentHeights[x];
            }
        }
    }

    return true;
}

void FieldPattern::setPattern(int x, int y, PatternType t, char variable, double score)
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
    case PatternType::VAR:
    case PatternType::MUST_VAR:
        CHECK('A' <= variable && variable <= 'Z');
        types_[x][y] = t;
        vars_[x][y] = variable;
        scores_[x][y] = score;
        heights_[x] = std::max(height(x), y);
        break;
    case PatternType::ALLOW_VAR:
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
PatternType FieldPattern::inferType(char c, PatternType typeForLowerCase)
{
    if (c == ' ' || c == '.')
        return PatternType::NONE;
    if (c == '*')
        return PatternType::ANY;
    if (c == '_')
        return PatternType::MUST_EMPTY;
    if ('A' <= c && c <= 'Z')
        return PatternType::VAR;
    if ('a' <= c && c <= 'z')
        return typeForLowerCase;

    return PatternType::NONE;
}

Position* FieldPattern::fillSameVariablePositions(int x, int y, char c, Position* positionQueueHead, FieldBitField* checked) const
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
            std::swap(pf.scores_[x][y], pf.scores_[7 - x][y]);
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
