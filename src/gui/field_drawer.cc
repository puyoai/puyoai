#include "gui/field_drawer.h"

#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <iomanip>
#include <memory>
#include <sstream>

#include <SDL.h>
#include <SDL_image.h>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/key.h"
#include "core/server/game_state.h"
#include "gui/main_window.h"
#include "gui/pixel_color.h"

DECLARE_string(data_dir);

static const string kJapaneseBdfName = "/jiskan24.bdf";
static const string kEnglishBdfName = "/12x24.bdf";
static const int kBdfSize = 24;

static const int PUYO_W = 32;
static const int PUYO_H = 32;

FieldDrawer::FieldDrawer() :
    backgroundSurface_(makeUniqueSDLSurface(IMG_Load((FLAGS_data_dir + "/assets/background.png").c_str()))),
    puyoSurface_(makeUniqueSDLSurface(IMG_Load((FLAGS_data_dir + "/assets/puyo.png").c_str()))),
    ojamaSurface_(makeUniqueSDLSurface(IMG_Load((FLAGS_data_dir + "/assets/yokoku.png").c_str()))),
    font_(nullptr)
{
    font_ = Kanji_OpenFont((FLAGS_data_dir + kJapaneseBdfName).c_str(), kBdfSize);
    Kanji_AddFont(font_, (FLAGS_data_dir + kEnglishBdfName).c_str());
    CHECK(font_ != NULL) << "Failed to load fonts";
}

FieldDrawer::~FieldDrawer()
{
}

void FieldDrawer::onInit()
{
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

    SDL_Rect bgRect = screen->mainBox().toSDLRect();
    SDL_BlitSurface(backgroundSurface_.get(), nullptr, screen->surface(), &bgRect);

    lock_guard<mutex> lock(mu_);
    drawField(screen, 0, gameState_->playerGameState(0));
    drawField(screen, 1, gameState_->playerGameState(1));
}

SDL_Rect FieldDrawer::toRect(PuyoColor pc)
{
    switch (pc) {
    case PuyoColor::RED:    return SDL_Rect { 0 * 32, 0, PUYO_W, PUYO_H };
    case PuyoColor::BLUE:   return SDL_Rect { 1 * 32, 0, PUYO_W, PUYO_H };
    case PuyoColor::YELLOW: return SDL_Rect { 2 * 32, 0, PUYO_W, PUYO_H };
    case PuyoColor::GREEN:  return SDL_Rect { 3 * 32, 0, PUYO_W, PUYO_H };
    case PuyoColor::OJAMA:  return SDL_Rect { 5 * 32, 0, PUYO_W, PUYO_H };
    default:
        break;
    }

    return SDL_Rect { 0, 0, 0, 0 };
}

void FieldDrawer::drawField(Screen* screen, int playerId, const PlayerGameState& pgs)
{
    SDL_Surface* surface = screen->surface();

    const Kumipuyo& kumipuyo = pgs.kumipuyoSeq.front();
    const KumipuyoPos& kumipuyoPos = pgs.kumipuyoPos;

    for (int x = 0; x < FieldConstant::MAP_WIDTH; ++x) {
        for (int y = 0; y < FieldConstant::MAP_HEIGHT; ++y) {
            PuyoColor c = pgs.field.color(x, y);
            if (pgs.playable) {
                if (x == kumipuyoPos.axisX() && y == kumipuyoPos.axisY())
                    c = kumipuyo.axis;
                if (x == kumipuyoPos.childX() && y == kumipuyoPos.childY())
                    c = kumipuyo.child;
            }

            Box b = BoundingBox::boxForDraw(playerId, x, y);
            b.moveOffset(screen->mainBox().sx, screen->mainBox().sy);
            SDL_Rect r = b.toSDLRect();
            if (isNormalColor(c) || c == PuyoColor::OJAMA) {
                SDL_Rect sourceRect = toRect(c);
                SDL_BlitSurface(puyoSurface_.get(), &sourceRect, surface, &r);
            }
        }
    }

    // Next puyo info
    static const NextPuyoPosition positions[] = {
        NextPuyoPosition::NEXT1_AXIS,
        NextPuyoPosition::NEXT1_CHILD,
        NextPuyoPosition::NEXT2_AXIS,
        NextPuyoPosition::NEXT2_CHILD,
    };
    for (int i = 0; i < 4; ++i) {
        Box b = BoundingBox::boxForDraw(playerId, positions[i]);
        b.moveOffset(screen->mainBox().sx, screen->mainBox().sy);
        SDL_Rect r = b.toSDLRect();
        PuyoColor c = pgs.kumipuyoSeq.color(positions[i]);
        if (isNormalColor(c) || c == PuyoColor::OJAMA) {
            SDL_Rect sourceRect = toRect(c);
            sourceRect.w = r.w;
            sourceRect.h = r.h;
            if (playerId == 1 && (i == 2 || i == 3)) {
                sourceRect.x += sourceRect.w;
            }

            SDL_BlitSurface(puyoSurface_.get(), &sourceRect, surface, &r);
        }
    }

    // Ojama
    int ojama = pgs.ojama();
    Box offsetBox = BoundingBox::boxForDraw(playerId, 1, 13);
    offsetBox.moveOffset(screen->mainBox().sx, screen->mainBox().sy);
    int offsetX = offsetBox.sx;
    int offsetY = offsetBox.sy;
    while (ojama > 0) {
        if (ojama >= 400) {
            SDL_Rect sourceRect { 0, 0, 32, 35 };
            SDL_Rect destRect { offsetX, offsetY, 32, 35 };
            SDL_BlitSurface(ojamaSurface_.get(), &sourceRect, surface, &destRect);
            offsetX += 32;
            ojama -= 400;
            continue;
        }

        if (ojama >= 300) {
            SDL_Rect sourceRect { 32, 0, 32, 35 };
            SDL_Rect destRect { offsetX, offsetY, 32, 35 };
            SDL_BlitSurface(ojamaSurface_.get(), &sourceRect, surface, &destRect);
            offsetX += 32;
            ojama -= 300;
            continue;
        }

        if (ojama >= 200) {
            SDL_Rect sourceRect { 64, 0, 32, 35 };
            SDL_Rect destRect { offsetX, offsetY, 32, 35 };
            SDL_BlitSurface(ojamaSurface_.get(), &sourceRect, surface, &destRect);
            offsetX += 32;
            ojama -= 200;
            continue;
        }

        if (ojama >= 30) {
            SDL_Rect sourceRect { 96, 0, 32, 35 };
            SDL_Rect destRect { offsetX, offsetY, 32, 35 };
            SDL_BlitSurface(ojamaSurface_.get(), &sourceRect, surface, &destRect);
            offsetX += 32;
            ojama -= 30;
            continue;
        }

        if (ojama >= 6) {
            SDL_Rect sourceRect { 128, 0, 28, 35 };
            SDL_Rect destRect { offsetX, offsetY, 28, 35 };
            SDL_BlitSurface(ojamaSurface_.get(), &sourceRect, surface, &destRect);
            offsetX += 28;
            ojama -= 6;
            continue;
        }

        if (ojama >= 1) {
            SDL_Rect sourceRect { 156, 0, 20, 35 };
            SDL_Rect destRect { offsetX, offsetY, 20, 35 };
            SDL_BlitSurface(ojamaSurface_.get(), &sourceRect, surface, &destRect);
            offsetX += 20;
            ojama -= 1;
            continue;
        }

        DCHECK(false) << "ojama = " << ojama;
    }

    SDL_Color white;
    white.r = white.g = white.b = 255;

    // Score
    {
        ostringstream ss;
        ss << setw(10) << pgs.score;
        Box b = BoundingBox::boxForDraw(playerId, 0, -1);
        b.moveOffset(screen->mainBox().sx, screen->mainBox().sy);
        Kanji_PutText(font_, b.sx, b.sy, surface, ss.str().c_str(), white);
    }
}
