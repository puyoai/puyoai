#ifndef CORE_POSITION_H_
#define CORE_POSITION_H_

// Position is used to represent puyo position.
struct Position {
    Position() {}
    constexpr Position(int x, int y) : x(x), y(y) {}

    int x;
    int y;
};

#endif
