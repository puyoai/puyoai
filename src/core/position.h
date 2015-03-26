#ifndef CORE_POSITION_H_
#define CORE_POSITION_H_

#include <ostream>

#include "base/slice.h"

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
    friend bool operator!=(const Position& lhs, const Position& rhs)
    {
        return !(lhs == rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const Position& p)
    {
        return os << '(' << p.x << ',' << p.y << ')';
    }

    int x;
    int y;
};

namespace std {

template<>
struct hash<Position>
{
    size_t operator()(const Position& p) const { return p.x * 16 + p.y; }
};

template<>
struct hash<Slice<Position>>
{
    size_t operator()(const Slice<Position>& vs) const
    {
        size_t h = 0;
        for (const auto& p : vs) {
            h = h * 37 + hash<Position>()(p);
        }
        return h;
    }
};

}

#endif
