#include "field.h"

#include <iostream>

#include "core/decision.h"
#include "core/field/field_bit_field.h"
#include "core/kumipuyo.h"
#include "core/position.h"

using namespace std;

void Field::dropKumipuyoSafely(const Decision& decision, const Kumipuyo& kumiPuyo)
{
    int x1 = decision.x;
    int x2 = decision.x + (decision.r == 1) - (decision.r == 3);

    PuyoColor c1 = kumiPuyo.axis;
    PuyoColor c2 = kumiPuyo.child;
    if (decision.r == 2)
        swap(c1, c2);

    if (height(x1) < 14)
        dropPuyoOn(x1, c1);
    if (height(x2) < 14)
        dropPuyoOn(x2, c2);
}

pair<int, int> Field::connectedPuyoNumsWithAllowingOnePointJump(int x, int y) const
{
    FieldBitField checked;
    return connectedPuyoNumsWithAllowingOnePointJump(x, y, checked);
}

pair<int, int> Field::connectedPuyoNumsWithAllowingOnePointJump(int x, int y, FieldBitField& checked) const
{
    DCHECK(isNormalColor(color(x, y)));
    Position positions[WIDTH * HEIGHT];
    int additional = 0;

    PuyoColor c = color(x, y);
    Position* end = fillSameColorPosition(x, y, c, positions, &checked);
    Position* current = end;

    for (Position* p = positions; p != end; ++p) {
        if (1 <= p->x - 2 && color(p->x - 1, p->y) == EMPTY && color(p->x - 2, p->y) == c && !checked.get(p->x - 2, p->y)) {
            Position* newCurrent = fillSameColorPosition(p->x - 2, p->y, c, current, &checked);
            if (newCurrent != current) {
                additional += 1;
                current = newCurrent;
            }
        }
        if (p->x + 2 <= Field::WIDTH && color(p->x + 1, p->y) == EMPTY && color(p->x + 2, p->y) == c && !checked.get(p->x + 2, p->y)) {
            Position* newCurrent = fillSameColorPosition(p->x + 2, p->y, c, current, &checked);
            if (newCurrent != current) {
                additional += 1;
                current = newCurrent;
            }
        }
    }

    return make_pair(current - positions, additional);
}

bool Field::findBestBreathingSpace(int& breathingX, int& breathingY, int x, int y) const
{
    DCHECK(color(x, y) != EMPTY);

    FieldBitField checked;
    Position positions[WIDTH * HEIGHT];

    Position* filledHead = fillSameColorPosition(x, y, color(x, y), positions, &checked);

    Position result;
    int resultHeight = 100;
    for (Position* p = positions; p != filledHead; ++p) {
        if (color(p->x, p->y + 1) == EMPTY) {
            result = Position(p->x, p->y + 1);
            resultHeight = 0;
            break;
        }

        if (color(p->x - 1, p->y) == EMPTY && p->y - height(p->x - 1) < resultHeight) {
            result = Position(p->x - 1, p->y);
            resultHeight = p->y - height(p->x - 1);
            if (resultHeight == 0)
                break;
        }

        if (color(p->x + 1, p->y) == EMPTY && p->y - height(p->x + 1) < resultHeight) {
            result = Position(p->x + 1, p->y);
            resultHeight = p->y - height(p->x + 1);
            if (resultHeight == 0)
                break;
        }
    }

    if (resultHeight >= 100)
        return false;

    breathingX = result.x;
    breathingY = result.y;
    return true;
}

void Field::showDebugOutput() const
{
    cerr << debugOutput() << endl;
}

