#ifndef CORE_PLAIN_FIELD_H_
#define CORE_PLAIN_FIELD_H_

#include <string>

#include "core/field_constant.h"
#include "core/puyo_color.h"

struct Position;
class FieldBitField;

class PlainField : public FieldConstant {
public:
    PlainField();
    explicit PlainField(const std::string& url);

    // Gets a color of puyo at a specified position.
    PuyoColor color(int x, int y) const { return field_[x][y]; }
    // TODO(mayah): Remove this from PlainField?
    void unsafeSet(int x, int y, PuyoColor c) { field_[x][y] = c; }

    // Returns true if puyo on (x, y) is c.
    bool isColor(int x, int y, PuyoColor c) const { return field_[x][y] == c; }
    // Returns true if puyo on (x, y) is empty.
    bool isEmpty(int x, int y) const { return isColor(x, y, PuyoColor::EMPTY); }

    // Returns true if the field does not have any puyo. This will return valid value
    // when some puyos are in the air.
    bool isZenkeshiPrecise() const;

    // Returns true if color(x, y) is connected in some direction.
    bool isConnectedPuyo(int x, int y) const;

    // Returns the number of puyos connected to (x, y).
    // Actually you can use this if color(x, y) is EMPTY or OJAMA.
    int countConnectedPuyos(int x, int y) const;
    // Same as countConnectedPuyos(x, y), but with checking using |checked|.
    int countConnectedPuyos(int x, int y, FieldBitField* checked) const;
    // Same as countConnectedPuyos(x, y).
    // If # of connected puyos is >= 4, the result is any value >= 4.
    // For example, if the actual number of connected is 6, result is 4, 5, or 6.
    // This is faster than countConnectedPuyos, so this will be useful when checking
    // puyo is vanished or not.
    int countConnectedPuyosMax4(int x, int y) const;

    // Drops all the puyos in the air. Puyos in the 14th row won't be dropped, though.
    void drop();

    // Inserts positions whose puyo color is the same as |c|, and connected to (x, y).
    // The checked cells will be marked in |checked|.
    // PositionQueueHead should have enough capacity.
    Position* fillSameColorPosition(int x, int y, PuyoColor c, Position* positionQueueHead, FieldBitField* checked) const;

    // Returns true if there is an empty neighbor of (x, y).
    bool hasEmptyNeighbor(int x, int y) const;

    std::string toString(char charIfEmpty = ' ') const;

    // Vanish.
    int vanishSlow(int currentChain);

    friend bool operator==(const PlainField&, const PlainField&);

private:
    void initialize();

    PuyoColor field_[MAP_WIDTH][MAP_HEIGHT];
};

inline bool PlainField::hasEmptyNeighbor(int x, int y) const
{
    return isEmpty(x, y + 1) || isEmpty(x, y - 1) || isEmpty(x + 1, y) || isEmpty(x - 1, y);
}

inline bool PlainField::isConnectedPuyo(int x, int y) const
{
    PuyoColor c = color(x, y);
    return color(x, y - 1) == c || color(x, y + 1) == c || color(x - 1, y) == c || color(x + 1, y) == c;
}

#endif
