#ifndef GUI_PIXEL_COLOR_H_
#define GUI_PIXEL_COLOR_H_

#include <SDL.h>
#include "core/puyo_color.h"
#include "core/real_color.h"

Uint32 toPixelColor(SDL_Surface*, PuyoColor);
Uint32 toPixelColor(SDL_Surface*, RealColor);

#endif
