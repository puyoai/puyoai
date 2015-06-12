#ifndef CAPTURE_REAL_COLOR_FIELD_H_
#define CAPTURE_REAL_COLOR_FIELD_H_

#include "core/field_constant.h"
#include "core/real_color.h"

class RealColorField : public FieldConstant {
public:
    RealColorField();
    explicit RealColorField(const std::string&);

    // TODO(mayah): Remove these methods.
    RealColor get(int x, int y) const { return field_[x][y]; }
    void set(int x, int y, RealColor rc) { field_[x][y] = rc; }

    RealColor color(int x, int y) const { return field_[x][y]; }
    void setColor(int x, int y, RealColor rc) { field_[x][y] = rc; }

private:
    RealColor field_[MAP_WIDTH][MAP_HEIGHT];
};

#endif
