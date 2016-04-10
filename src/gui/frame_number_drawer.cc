#include "gui/frame_number_drawer.h"

#include <cstdint>
#include <sstream>
#include <string>

#include <SDL_ttf.h>

#include "base/strings.h"

#include "gui/screen.h"
#include "gui/unique_sdl_surface.h"

using namespace std;

void FrameNumberDrawer::draw(Screen* screen)
{
    ++frameNumber_;

    stringstream ss;
    if (screen->surface()->userdata != 0) {
        // When userdata exists, it should be frame id.
        uintptr_t v = reinterpret_cast<uintptr_t>(screen->surface()->userdata);
        ss << "Frame " << v;
    } else {
        ss << "Frame " << frameNumber_;
    }

    SDL_Color c;
    c.r = c.g = c.b = 255;
    c.a = 255;

    UniqueSDLSurface surf(makeUniqueSDLSurface(TTF_RenderUTF8_Blended(screen->font(), ss.str().c_str(), c)));
    if (!surf.get()) {
        LOG(ERROR) << "Failed to write text?";
        return;
    }

    SDL_Rect dr = {
        static_cast<Sint16>(screen->surface()->w / 2 - surf->w / 2),
        static_cast<Sint16>(screen->surface()->h - surf->h - 8),
        0,
        0
    };
    SDL_BlitSurface(surf.get(), NULL, screen->surface(), &dr);
}
