#ifndef CORE_POSITION_H_
#define CORE_POSITION_H_

#include <ostream>

// Position is used to represent puyo position.
struct Position {
    Position() {}
    constexpr Position(int x, int y) : x(x), y(y) {}
    Position(const Position& position) : x(position.x), y(position.y) {}

    friend bool operator<(const Position& lhs, const Position& rhs)
    {
        return lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y);
    }

    friend bool operator==(const Position& lhs, const Position& rhs)
    {
        return lhs.x == rhs.x && lhs.y == rhs.y;
    }

    friend std::ostream& operator<<(std::ostream& os, const Position& p)
    {
        return os << '(' << p.x << ',' << p.y << ')';
    }

    int x;
    int y;
};

#endif
