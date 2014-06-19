#include "gui/field_drawer.h"

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <iomanip>
#include <memory>
#include <sstream>

#include <SDL.h>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/key.h"
#include "core/state.h"
#include "duel/field_realtime.h"
#include "duel/game_state.h"
#include "gui/main_window.h"
#include "gui/pixel_color.h"

DECLARE_string(data_dir);

static const string kJapaneseBdfName = "/jiskan24.bdf";
static const string kEnglishBdfName = "/12x24.bdf";
static const int kBdfSize = 24;

static const int PUYO_W = 20;
static const int PUYO_H = 20;

FieldDrawer::FieldDrawer() :
    font_(nullptr)
{
    font_ = Kanji_OpenFont((FLAGS_data_dir + kJapaneseBdfName).c_str(), kBdfSize);
    Kanji_AddFont(font_, (FLAGS_data_dir + kEnglishBdfName).c_str());
    CHECK(font_ != NULL) << "Failed to load fonts";
}

FieldDrawer::~FieldDrawer()
{
}

void FieldDrawer::onInit(Screen* screen)
{
    BoundingBox::instance().setGenerator(screen->mainBox().sx + 50, screen->mainBox().sy + 100, PUYO_W, PUYO_H);

    ColorMap::instance().setColorMap(EMPTY,  RC_EMPTY);
    ColorMap::instance().setColorMap(OJAMA,  RC_OJAMA);
    ColorMap::instance().setColorMap(RED,    RC_RED);
    ColorMap::instance().setColorMap(BLUE,   RC_BLUE);
    ColorMap::instance().setColorMap(YELLOW, RC_YELLOW);
    ColorMap::instance().setColorMap(GREEN,  RC_GREEN);
}

void FieldDrawer::onUpdate(const GameState& gameState)
{
    lock_guard<mutex> lock(mu_);
    gameState_.reset(new GameState(gameState));
}

void FieldDrawer::draw(Screen* screen)
{
    if (!gameState_)
        return;

    lock_guard<mutex> lock(mu_);
    drawField(screen, gameState_->field(0));
    drawField(screen, gameState_->field(1));
}

void FieldDrawer::drawField(Screen* screen, const FieldRealtime& field)
{
    SDL_Surface* surface = screen->surface();

    Kumipuyo kumipuyo = field.kumipuyo();
    KumipuyoPos kumipuyoPos = field.kumipuyoPos();

    for (int x = 0; x < CoreField::MAP_WIDTH; ++x) {
        for (int y = 0; y < CoreField::MAP_HEIGHT; ++y) {
            Box b = BoundingBox::instance().get(field.playerId(), x, y);
            SDL_Rect r = b.toSDLRect();

            PuyoColor c = field.field().color(x, y);
            if (field.userPlayable()) {
                if (x == kumipuyoPos.axisX() && y == kumipuyoPos.axisY())
                    c = puyoColorOf(kumipuyo.axis);
                if (x == kumipuyoPos.childX() && y == kumipuyoPos.childY())
                    c = puyoColorOf(kumipuyo.child);
            }

            SDL_FillRect(surface, &r, toPixelColor(surface, c));
        }
    }

    // Death line
    {
        Box b1 = BoundingBox::instance().get(field.playerId(), 0, 12);
        Box b2 = BoundingBox::instance().get(field.playerId(), 7, 12);

        SDL_Rect r;
        r.x = b1.sx;
        r.y = b1.sy;
        r.w = b2.dx - b1.sx;
        r.h = 1;
        SDL_FillRect(surface, &r, SDL_MapRGB(surface->format, 255, 255, 255));
    }

    // Next puyo info
    const NextPuyoPosition positions[] = {
        NextPuyoPosition::NEXT1_AXIS,
        NextPuyoPosition::NEXT1_CHILD,
        NextPuyoPosition::NEXT2_AXIS,
        NextPuyoPosition::NEXT2_CHILD,
    };
    for (int i = 0; i < 4; ++i) {
        SDL_Rect r = BoundingBox::instance().get(field.playerId(), positions[i]).toSDLRect();
        Uint32 c = toPixelColor(surface, field.puyoColor(positions[i]));
        SDL_FillRect(surface, &r, c);
    }

    SDL_Color white;
    white.r = white.g = white.b = 255;

    // Score
    {
        ostringstream ss;
        ss << setw(10) << field.score();
        Box b = BoundingBox::instance().get(field.playerId(), 0, -1);
        Kanji_PutText(font_,
                      b.sx, b.sy,
                      surface, ss.str().c_str(), white);
    }

    // Ojama
    {
        ostringstream ss;
        ss << field.numFixedOjama() << '(' << field.numPendingOjama() << ')';
        Box b = BoundingBox::instance().get(field.playerId(), 0, 16);
        Kanji_PutText(font_,
                      b.sx, b.sy,
                      surface, ss.str().c_str(), white);
    }
}
