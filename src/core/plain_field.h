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
    void unsafeSet(int x, int y, PuyoColor c) { field_[x][y] = c; }

    std::string toString(char charIfEmpty = ' ') const;
private:
    void initialize();

    PuyoColor field_[MAP_WIDTH][MAP_HEIGHT];
};

#endif
