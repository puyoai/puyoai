#ifndef CORE_PLAIN_FIELD_H_
#define CORE_PLAIN_FIELD_H_

#include <string>

#include "core/field_constant.h"
#include "core/puyo_color.h"

class PlainField : public FieldConstant {
public:
    PlainField();
    PlainField(const std::string& url);

    PuyoColor get(int x, int y) const { return field_[x][y]; }
    void unsafeSet(int x, int y, PuyoColor c) { field_[x][y] = c; }

    std::string toString(char charIfEmpty = ' ') const;

    // True when visible field is the same
    bool isVisibllySame(const PlainField&) const;
    bool canBeOverriden(const PlainField&) const;

private:
    void initialize();

    PuyoColor field_[MAP_WIDTH][MAP_HEIGHT];
};

#endif
