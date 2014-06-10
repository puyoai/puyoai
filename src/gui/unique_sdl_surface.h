#ifndef GUI_UNIQUE_SURFACE_H_
#define GUI_UNIQUE_SURFACE_H_

#include <memory>
#include <SDL.h>

typedef std::unique_ptr<SDL_Surface, void (*)(SDL_Surface*)> UniqueSDLSurface;

inline UniqueSDLSurface makeUniqueSDLSurface(SDL_Surface* surface)
{
    return UniqueSDLSurface(surface, SDL_FreeSurface);
}

inline UniqueSDLSurface emptyUniqueSDLSurface()
{
    return UniqueSDLSurface(nullptr, SDL_FreeSurface);
}

#endif
