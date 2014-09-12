#ifndef CORE_PLAIN_FIELD_H_
#define CORE_PLAIN_FIELD_H_

#include <string>
#include "core/puyo_color.h"

class PlainField {
public:
    static const int WIDTH = 6;
    static const int HEIGHT = 12;
    static const int MAP_WIDTH = 1 + WIDTH + 1;
    static const int MAP_HEIGHT = 1 + HEIGHT + 3;

    PlainField();
    PlainField(const std::string& url);

    PuyoColor get(int x, int y) const { return field_[x][y]; }
    void unsafeSet(int x, int y, PuyoColor c) { field_[x][y] = c; }

    std::string toString(char charIfEmpty = ' ') const;

private:
    void initialize();

    PuyoColor field_[MAP_WIDTH][MAP_HEIGHT];
};

#endif
