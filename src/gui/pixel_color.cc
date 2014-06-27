#include "gui/pixel_color.h"

Uint32 toPixelColor(SDL_Surface* surface, PuyoColor pc)
{
    switch (pc) {
    case PuyoColor::EMPTY:
        return SDL_MapRGB(surface->format,   0,   0,   0);
    case PuyoColor::OJAMA:
        return SDL_MapRGB(surface->format, 127, 127, 127);
    case PuyoColor::WALL:
        return SDL_MapRGB(surface->format, 255, 255, 255);
    case PuyoColor::RED:
        return SDL_MapRGB(surface->format, 255,   0,   0);
    case PuyoColor::BLUE:
        return SDL_MapRGB(surface->format,   0,   0, 255);
    case PuyoColor::YELLOW:
        return SDL_MapRGB(surface->format, 255, 255,   0);
    case PuyoColor::GREEN:
        return SDL_MapRGB(surface->format,   0, 255,   0);
    }

    DCHECK(false) << "Invalid PuyoColor: " << static_cast<int>(pc);
    return SDL_MapRGB(surface->format, 0, 0, 0);
}

Uint32 toPixelColor(SDL_Surface* surface, RealColor rc)
{
    switch (rc) {
    case RealColor::RC_EMPTY:
        return SDL_MapRGB(surface->format,   0,   0,   0);
    case RealColor::RC_OJAMA:
        return SDL_MapRGB(surface->format, 255, 255, 255);
    case RealColor::RC_RED:
        return SDL_MapRGB(surface->format, 255,   0,   0);
    case RealColor::RC_BLUE:
        return SDL_MapRGB(surface->format,   0,   0, 255);
    case RealColor::RC_YELLOW:
        return SDL_MapRGB(surface->format, 255, 255,   0);
    case RealColor::RC_GREEN:
        return SDL_MapRGB(surface->format,   0, 255,   0);
    case RealColor::RC_PURPLE:
        return SDL_MapRGB(surface->format, 255,   0, 255);
    }

    DCHECK(false) << "Invalid PuyoColor " << static_cast<int>(rc);
    return SDL_MapRGB(surface->format, 127, 127, 127);
}
