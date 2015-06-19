#include "gui/decision_drawer.h"

#include "core/server/game_state.h"
#include "gui/SDL_prims.h"
#include "gui/pixel_color.h"

using namespace std;

void DecisionDrawer::onUpdate(const GameState& gameState)
{
    lock_guard<mutex> lock(mu_);

    for (int pi = 0; pi < 2; ++pi) {
        const PlayerGameState& pgs = gameState.playerGameState(pi);
        if (pgs.kumipuyoSeq.size() >= 1) {
            pos_[pi] = pgs.kumipuyoPos;
            kumipuyo_[pi] = pgs.kumipuyoSeq.front();
        } else {
            pos_[pi] = KumipuyoPos();
            kumipuyo_[pi] = Kumipuyo();
        }
    }
}

void DecisionDrawer::draw(Screen* screen)
{
    KumipuyoPos pos[2];
    Kumipuyo kumipuyo[2];

    {
        lock_guard<mutex> lock(mu_);
        for (int pi = 0; pi < 2; ++pi) {
            pos[pi] = pos_[pi];
            kumipuyo[pi] = kumipuyo_[pi];
        }
    }

    SDL_Surface* surface = screen->surface();

    for (int pi = 0; pi < 2; ++pi) {
        if (!pos[pi].isValid() || !kumipuyo[pi].isValid())
            continue;

        Box b1 = BoundingBox::boxForDraw(pi, pos[pi].axisX(), pos[pi].axisY());
        b1.moveOffset(screen->mainBox().sx, screen->mainBox().sy);
        if (isNormalColor(kumipuyo[pi].axis)) {
            int xx = (b1.sx + b1.dx) / 2;
            int yy = (b1.sy + b1.dy) / 2;
            int r = b1.w() / 2 - 1;
            // TODO(mayah): Use RealColor instead of PuyoColor::OJAMA.
            Uint32 c = toPixelColor(surface, RealColor::RC_OJAMA);
            SDL_DrawCircle(surface, xx, yy, r, c);
        }

        Box b2 = BoundingBox::boxForDraw(pi, pos[pi].childX(), pos[pi].childY());
        b2.moveOffset(screen->mainBox().sx, screen->mainBox().sy);
        if (isNormalColor(kumipuyo[pi].child)) {
            int xx = (b2.sx + b2.dx) / 2;
            int yy = (b2.sy + b2.dy) / 2;
            int r = b2.w() / 2 - 1;
            Uint32 c = toPixelColor(surface, RealColor::RC_OJAMA);
            SDL_DrawCircle(surface, xx, yy, r, c);
        }
    }
}
