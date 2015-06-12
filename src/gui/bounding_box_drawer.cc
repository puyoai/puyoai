#include "gui/bounding_box_drawer.h"

#include <glog/logging.h>

#include "gui/SDL_prims.h"
#include "gui/screen.h"

void BoundingBoxDrawer::draw(Screen* screen)
{
    SDL_Surface* surface = screen->surface();

    Uint32 color = SDL_MapRGB(surface->format, 255, 255, 255);

    CHECK(SDL_LockSurface(surface) == 0);
    for (int pi = 0; pi < 2; ++pi) {
        for (int x = 1; x <= 6; ++x) {
            for (int y = 1; y <= 12; ++y) {
                Box b = BoundingBox::boxForDraw(pi, x, y);
                SDL_DrawLine(surface, b.sx, b.sy, b.dx, b.sy, color);
                SDL_DrawLine(surface, b.sx, b.sy, b.sx, b.dy, color);
                SDL_DrawLine(surface, b.dx, b.sy, b.dx, b.dy, color);
                SDL_DrawLine(surface, b.sx, b.dy, b.dx, b.dy, color);
            }
        }
    }

    for (int pi = 0; pi < 2; ++pi) {
        Box bs[4] = {
            BoundingBox::boxForDraw(pi, NextPuyoPosition::NEXT1_AXIS),
            BoundingBox::boxForDraw(pi, NextPuyoPosition::NEXT1_CHILD),
            BoundingBox::boxForDraw(pi, NextPuyoPosition::NEXT2_AXIS),
            BoundingBox::boxForDraw(pi, NextPuyoPosition::NEXT2_CHILD),
        };

        for (int i = 0; i < 4; ++i) {
            Box b = bs[i];
            SDL_DrawLine(surface, b.sx, b.sy, b.dx, b.sy, color);
            SDL_DrawLine(surface, b.sx, b.sy, b.sx, b.dy, color);
            SDL_DrawLine(surface, b.dx, b.sy, b.dx, b.dy, color);
            SDL_DrawLine(surface, b.sx, b.dy, b.dx, b.dy, color);
        }
    }

    {
        Box b = BoundingBox::boxForDraw(BoundingBox::Region::LEVEL_SELECT_1P);
        SDL_DrawLine(surface, b.sx, b.sy, b.dx, b.sy, color);
        SDL_DrawLine(surface, b.sx, b.sy, b.sx, b.dy, color);
        SDL_DrawLine(surface, b.dx, b.sy, b.dx, b.dy, color);
        SDL_DrawLine(surface, b.sx, b.dy, b.dx, b.dy, color);
    }
    {
        Box b = BoundingBox::boxForDraw(BoundingBox::Region::LEVEL_SELECT_2P);
        SDL_DrawLine(surface, b.sx, b.sy, b.dx, b.sy, color);
        SDL_DrawLine(surface, b.sx, b.sy, b.sx, b.dy, color);
        SDL_DrawLine(surface, b.dx, b.sy, b.dx, b.dy, color);
        SDL_DrawLine(surface, b.sx, b.dy, b.dx, b.dy, color);
    }

    SDL_UnlockSurface(surface);
}
