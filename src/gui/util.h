#ifndef GUI_UTIL_H_
#define GUI_UTIL_H_

#include <SDL.h>

Uint32 getpixel(const SDL_Surface* surface, int x, int y);
void putpixel(SDL_Surface* surface, int x, int y, Uint32 pixel);

#endif

