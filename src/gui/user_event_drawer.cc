#include "gui/user_event_drawer.h"

#include "core/server/game_state.h"
#include "gui/bounding_box.h"
#include "gui/box.h"
#include "gui/screen.h"
#include "gui/unique_sdl_surface.h"

using namespace std;

void UserEventDrawer::onUpdate(const GameState& gameState)
{
    lock_guard<mutex> lock(mu_);
    userEvents_[0] = gameState.playerGameState(0).event;
    userEvents_[1] = gameState.playerGameState(1).event;
}

void UserEventDrawer::draw(Screen* screen)
{
    SDL_Color c;
    c.r = c.g = c.b = 0;
    c.a = 255;

    for (int i = 0; i < 2; ++i) {
        string buf(userEvents_[i].toString());
        UniqueSDLSurface surf(makeUniqueSDLSurface(TTF_RenderUTF8_Blended(screen->font(), buf.c_str(), c)));
        if (!surf.get())
            continue;

        Box b1 = BoundingBox::instance().get(i, 1, 12);
        Box b2 = BoundingBox::instance().get(i, 6, 12);
        Box b(b1.sx, b1.sy, b2.dx, b2.dy);
        b.moveOffset(screen->mainBox().sx, screen->mainBox().sy);

        SDL_Rect dr = {
            static_cast<Sint16>(b.sx + b.w() / 2 - surf->w / 2),
            static_cast<Sint16>(surf->h / 2),
            0,
            0
        };

        SDL_BlitSurface(surf.get(), NULL, screen->surface(), &dr);
    }
}
