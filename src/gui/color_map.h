#ifndef GUI_COLOR_MAP_H_
#define GUI_COLOR_MAP_H_

#include <SDL.h>

#include "base/base.h"
#include "base/lock.h"
#include "core/puyo_color.h"
#include "core/real_color.h"

class ColorMap : private noncopyable {
public:
    static ColorMap& instance();

    void setColorMap(PuyoColor, RealColor);

    PuyoColor toPuyoColor(RealColor) const;
    RealColor toRealColor(PuyoColor) const;

    Uint32 getColor(PuyoColor c) const;
    Uint32 getColor(RealColor rc) const;

private:
    ColorMap();
    ~ColorMap();

    Mutex mu_;
    RealColor realColors_[10];
    PuyoColor puyoColors_[10];

    SDL_Color sdl_colors_[10];
    Uint32 colors_[10];

    Uint32 realPuyoColors_[NUM_PUYO_REAL_COLOR];
};

#endif
