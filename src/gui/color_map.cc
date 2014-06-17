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
    for (int i = 0; i < 10; ++i) {
        realColors_[i] = RealColor::RC_INVALID;
        puyoColors_[i] = INVALID;
    }

    memset(sdl_colors_, 0, sizeof(sdl_colors_));
    memset(colors_, 0, sizeof(colors_));

    UniqueSDLSurface s(makeUniqueSDLSurface(SDL_CreateRGBSurface(0, 1, 1, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000)));
    static int COLORS[10][3] = {
        { 0, 0, 0 },
        { 0, 255, 255 },
        { 0, 0, 0 },
        { 255, 255, 255 },
        { 255, 0, 0 },
        { 0, 0, 255 },
        { 255, 255, 0 },
        { 0, 255, 255 },
        { 255, 0, 255 },
        { 0, 0, 0 }
    };

    for (int i = 0; i < 10; i++) {
        int* c = COLORS[i];
        sdl_colors_[i].r = c[0];
        sdl_colors_[i].g = c[1];
        sdl_colors_[i].b = c[2];
        colors_[i] = SDL_MapRGB(s->format, c[0], c[1], c[2]);
    }

    realPuyoColors_[RealColor::RC_EMPTY]   = SDL_MapRGB(s->format,   0,   0,   0);
    realPuyoColors_[RealColor::RC_OJAMA]   = SDL_MapRGB(s->format, 255, 255, 255);
    realPuyoColors_[RealColor::RC_RED]     = SDL_MapRGB(s->format, 255,   0,   0);
    realPuyoColors_[RealColor::RC_BLUE]    = SDL_MapRGB(s->format,   0,   0, 255);
    realPuyoColors_[RealColor::RC_YELLOW]  = SDL_MapRGB(s->format, 255, 255,   0);
    realPuyoColors_[RealColor::RC_GREEN]   = SDL_MapRGB(s->format,   0, 255,   0);
    realPuyoColors_[RealColor::RC_PURPLE]  = SDL_MapRGB(s->format, 255,   0, 255);
    realPuyoColors_[RealColor::RC_INVALID] = SDL_MapRGB(s->format, 127, 127, 127);
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

Uint32 ColorMap::getColor(PuyoColor c) const
{
    return colors_[static_cast<int>(c)];
}

Uint32 ColorMap::getColor(RealColor rc) const
{
    int idx = static_cast<int>(rc);
    DCHECK(0 <= idx && idx < NUM_PUYO_REAL_COLOR) << idx;
    return realPuyoColors_[idx];
}
