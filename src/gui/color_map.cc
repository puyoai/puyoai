#include "gui/color_map.h"

#include "gui/unique_sdl_surface.h"

// static
ColorMap& ColorMap::instance()
{
    static ColorMap m;
    return m;
}

ColorMap::ColorMap()
{
    for (int i = 0; i < NUM_PUYO_COLORS; ++i)
        realColors_[i] = RealColor::RC_INVALID;
    for (int i = 0; i < NUM_REAL_COLORS; ++i)
        puyoColors_[i] = PuyoColor::INVALID;
}

ColorMap::~ColorMap()
{
}

void ColorMap::setColorMap(PuyoColor pc, RealColor rc)
{
    DCHECK(pc != INVALID);
    DCHECK(rc != RealColor::RC_INVALID);
    DCHECK(realColors_[static_cast<int>(pc)] == RealColor::RC_INVALID);
    DCHECK(puyoColors_[static_cast<int>(rc)] == INVALID);

    realColors_[static_cast<int>(pc)] = rc;
    puyoColors_[static_cast<int>(rc)] = pc;
}

PuyoColor ColorMap::toPuyoColor(RealColor rc) const
{
    int idx = static_cast<int>(rc);
    DCHECK(0 <= idx && idx < 10) << idx;
    return puyoColors_[idx];
}

RealColor ColorMap::toRealColor(PuyoColor pc) const
{
    int idx = static_cast<int>(pc);
    DCHECK(0 <= idx && idx < 10) << idx;
    return realColors_[idx];
}
