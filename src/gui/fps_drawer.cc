#include "gui/fps_drawer.h"

#include <string>

#include <SDL_ttf.h>

#include "base/strings.h"

#include "gui/screen.h"
#include "gui/unique_sdl_surface.h"

using namespace std;

FPSDrawer::FPSDrawer() :
    frames_(0),
    ticks_ {}
{
}

FPSDrawer::~FPSDrawer()
{
}

void FPSDrawer::draw(Screen* screen)
{
    Uint32 prevTicks = ticks_[frames_ % 30];
    Uint32 currentTicks = SDL_GetTicks();

    Uint32 elapsed = currentTicks - prevTicks;
    ticks_[frames_++ % 30] = currentTicks;

    // No time elapsed? Weird.
    if (elapsed == 0)
        return;

    int fps = 30 * 1000 / elapsed;
    string buf = to_string(fps) + " fps";

    SDL_Color c;
    c.r = c.g = c.b = 0;
    c.a = 255;

    UniqueSDLSurface surf(makeUniqueSDLSurface(TTF_RenderUTF8_Blended(screen->font(), buf.c_str(), c)));
    if (surf.get()) {
        SDL_Rect dr = {
            static_cast<Sint16>(screen->surface()->w / 2 - surf->w / 2),
            static_cast<Sint16>(surf->h / 2),
            0,
            0
        };
        SDL_BlitSurface(surf.get(), NULL, screen->surface(), &dr);
    }
}
