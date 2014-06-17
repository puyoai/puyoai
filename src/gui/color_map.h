#ifndef GUI_COLOR_MAP_H_
#define GUI_COLOR_MAP_H_

#include <mutex>
#include <SDL.h>

#include "base/base.h"
#include "core/puyo_color.h"
#include "core/real_color.h"

class ColorMap : private noncopyable {
public:
    static ColorMap& instance();

    void setColorMap(PuyoColor, RealColor);

    PuyoColor toPuyoColor(RealColor) const;
    RealColor toRealColor(PuyoColor) const;

private:
    ColorMap();
    ~ColorMap();

    std::mutex mu_;
    RealColor realColors_[NUM_PUYO_COLORS];
    PuyoColor puyoColors_[NUM_REAL_COLORS];
};

#endif
