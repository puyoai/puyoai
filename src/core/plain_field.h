#ifndef CORE_PLAIN_FIELD_H_
#define CORE_PLAIN_FIELD_H_

#include <string>

#include "core/field_constant.h"
#include "core/puyo_color.h"

class PlainField : public FieldConstant {
public:
    PlainField();
    explicit PlainField(const std::string& url);

    // Gets a color of puyo at a specified position.
    PuyoColor color(int x, int y) const { return field_[x][y]; }
    // Returns true if puyo on (x, y) is c.
    bool isColor(int x, int y, PuyoColor c) const { return field_[x][y] == c; }
    // Returns true if puyo on (x, y) is empty.
    bool isEmpty(int x, int y) const { return isColor(x, y, PuyoColor::EMPTY); }

    // Returns true if there is an empty neighbor of (x, y).
    bool hasEmptyNeighbor(int x, int y) const;

    // TODO(mayah): Remove this from PlainField.
    void unsafeSet(int x, int y, PuyoColor c) { field_[x][y] = c; }

    std::string toString(char charIfEmpty = ' ') const;

private:
    void initialize();

    PuyoColor field_[MAP_WIDTH][MAP_HEIGHT];
};

inline bool PlainField::hasEmptyNeighbor(int x, int y) const
{
    return isEmpty(x, y + 1) || isEmpty(x, y - 1) || isEmpty(x + 1, y) || isEmpty(x - 1, y);
}

#endif
