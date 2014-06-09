#include "gui/commentator.h"

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include <glog/logging.h>
#include <glog/stl_logging.h>

#include "base/base.h"
#include "core/algorithm/plan.h"
#include "core/algorithm/rensa_detector.h"
#include "core/state.h"
#include "duel/game_state.h"
#include "duel/field_realtime.h"
#include "gui/color_map.h"
#include "gui/screen.h"
#include "gui/unique_sdl_surface.h"
#include "gui/util.h"

using namespace std;

namespace {

static void drawDiamond(Screen* screen, const Box& b, PuyoColor c)
{
    Uint32 col = ColorMap::instance().getColor(c);
    SDL_Surface* surface = screen->surface();

    int w = (b.dx - b.sx - 2) / 2;
    int cx = (b.sx + b.dx) / 2;
    int cy = (b.sy + b.dy) / 2;
    for (int i = 0; i < w; i++) {
        putpixel(surface, cx + i, cy - w + i, col);
        putpixel(surface, cx + w - i, cy + i, col);
        putpixel(surface, cx - i, cy + w - i, col);
        putpixel(surface, cx - w + i, cy - i, col);
    }
}

static void drawTriangle(Screen* screen, const Box& b, PuyoColor c)
{
    Uint32 col = ColorMap::instance().getColor(c);
    SDL_Surface* surface = screen->surface();

    int w = (b.dx - b.sx - 2);
    int cx = (b.sx + b.dx) / 2;
    int cy = (b.sy + b.dy) / 2;
    for (int i = 0; i < w; i++) {
        putpixel(surface, cx - w / 2 + i, cy + w / sqrt(3) / 2, col);
        putpixel(surface, cx + i / 2, cy - w / sqrt(3) + i * sqrt(3) / 2, col);
        putpixel(surface, cx - i / 2, cy - w / sqrt(3) + i * sqrt(3) / 2, col);
    }
}

static void drawNumber(Screen* screen, const Box& b, int n)
{
    SDL_Surface* surface = screen->surface();
    SDL_Color color;
    color.r = color.g = color.b = 255;

    ostringstream oss;
    oss << n;

    UniqueSDLSurface s(makeUniqueSDLSurface(TTF_RenderUTF8_Blended(screen->font(), oss.str().c_str(), color)));

    SDL_Rect dr;
    dr.x = b.dx - s->w;
    dr.y = b.dy - s->h;
    dr.x += 3;
    dr.w = 8;
    dr.h = 20;

    SDL_FillRect(surface, &dr, SDL_MapRGB(s->format, 1, 1, 1));
    SDL_BlitSurface(s.get(), NULL, surface, &dr);
}

static void drawText(Screen* screen, const char* msg, int x, int y)
{
    if (!*msg)
        return;

    SDL_Surface* surface = screen->surface();

    SDL_Color fg;
    fg.r = fg.g = fg.b = 0;
    UniqueSDLSurface s(makeUniqueSDLSurface(TTF_RenderUTF8_Shaded(screen->font(), msg, fg, screen->bgColor())));

    SDL_Rect dr;
    dr.x = x;
    dr.y = y;
    if (surface->w < x + s->w)
        dr.x = surface->w - s->w;

    SDL_BlitSurface(s.get(), NULL, surface, &dr);
}

static void drawText(Screen* screen, const std::string& str, int x, int y)
{
    drawText(screen, str.c_str(), x, y);
}

static void drawTrace(Screen* screen, int pi, const RensaTrackResult& result)
{
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 13; ++y) {
            int n = result.erasedAt(x, y);
            if (!n)
                continue;
            drawNumber(screen, BoundingBox::instance().get(pi, x, y), n);
        }
    }
}

}  // anonymous namespace

static void* runCommentatorLoop(void* d)
{
    reinterpret_cast<Commentator*>(d)->runLoop();
    return nullptr;
}

Commentator::Commentator() :
    shouldStop_(false)
{
    needsUpdate_[0] = needsUpdate_[1] = false;
}

Commentator::~Commentator()
{
    // TODO(mayah): Why don't we have stop() instead of calling pthread_join() here?
    shouldStop_ = true;
    pthread_join(th_, nullptr);
}

void Commentator::newGameWillStart()
{
    reset();
}

void Commentator::onUpdate(const GameState& gameState)
{
    ScopedLock lock(&mu_);

    for (int i = 0; i < 2; ++i) {
        const FieldRealtime& rf = gameState.field(i);
        if (rf.userState().grounded) {
            field_[i] = rf.field();
            // When chigiri is used, some puyo exists in the air. So we need to drop.
            field_[i].forceDrop();
            kumipuyoSeq_[i] = rf.kumipuyoSeq();
            needsUpdate_[i] = true;
        }

        string msg = gameState.message(i);
        if (msg != "")
            message_[i] = msg;
    }
}

bool Commentator::start()
{
    return pthread_create(&th_, nullptr, runCommentatorLoop, this) == 0;
}

void Commentator::stop()
{
    shouldStop_ = true;
}

void Commentator::runLoop()
{
     while (!shouldStop_) {
         SDL_Delay(16);
         for (int pi = 0; pi < 2; ++pi) {
             // Since we don't want to lock for long, copy field and kumipuyo.
             CoreField field;
             KumipuyoSeq kumipuyoSeq;
             {
                 ScopedLock lock(&mu_);
                 if (!needsUpdate_[pi])
                     continue;

                 field = field_[pi];
                 kumipuyoSeq = kumipuyoSeq_[pi];
                 needsUpdate_[pi] = false;
             }

             update(pi, field, kumipuyoSeq);
         }
     }
}

void Commentator::update(int pi, const CoreField& field, const KumipuyoSeq& kumipuyoSeq)
{
    // 1. Check field is firing a rensa.
    {
        CoreField f(field);
        unique_ptr<TrackedPossibleRensaInfo> track(new TrackedPossibleRensaInfo);
        track->rensaInfo = f.simulateAndTrack(&track->trackResult);
        if (track->rensaInfo.score > 0) {
            ScopedLock lock(&mu_);
            string msg = std::to_string(track->rensaInfo.chains) + "連鎖発火: " + std::to_string(track->rensaInfo.score) + "点";
            events_[pi].push_back(msg);
            firingChain_[pi] = move(track);
            fireableMainChain_[pi].reset();
            fireableTsubushiChain_[pi].reset();
            return;
        }

        ScopedLock lock(&mu_);
        firingChain_[pi].reset();
    }

    // 2. Check Tsubushi chain
    {
        KumipuyoSeq kp(kumipuyoSeq);
        if (3 < kp.size())
            kp.resize(3);

        vector<Plan> plans = Plan::findAvailablePlans(field, kp);
        double bestTsubushiScore = 0.0;
        const Plan* bestTsubushiPlan = nullptr;
        for (const auto& plan : plans) {
            if (!plan.isRensaPlan())
                continue;
            if (3 < plan.chains())
                continue;
            if (plan.score() < 200)
                continue;

            double tsubushiScore = static_cast<double>(plan.score()) / plan.totalFrames();
            if (bestTsubushiScore < tsubushiScore) {
                bestTsubushiScore = tsubushiScore;
                bestTsubushiPlan = &plan;
            }
        }

        if (bestTsubushiPlan != nullptr) {
            ScopedLock lock(&mu_);
            fireableTsubushiChain_[pi].reset(new Plan(*bestTsubushiPlan));
        }
    }

    // 3. Check Main chain
    {
        vector<TrackedPossibleRensaInfo> rs = RensaDetector::findPossibleRensasWithTracking(field, 3);
        const TrackedPossibleRensaInfo* bestRensa = nullptr;
        int maxScore = 0;
        for (const auto& r : rs) {
            if (maxScore < r.rensaInfo.score) {
                maxScore = r.rensaInfo.score;
                bestRensa = &r;
            }
        }

        if (bestRensa != nullptr) {
            ScopedLock lock(&mu_);
            fireableMainChain_[pi].reset(new TrackedPossibleRensaInfo(*bestRensa));
        }
    }
}

void Commentator::reset()
{
    ScopedLock lock(&mu_);
    needsUpdate_[0] = needsUpdate_[1] = false;
    for (int i = 0; i < 2; i++) {
        fireableMainChain_[i].reset();
        fireableTsubushiChain_[i].reset();
        firingChain_[i].reset();
    }
}

// ----------------------------------------------------------------------

void Commentator::draw(Screen* screen)
{
    ScopedLock lock(&mu_);
    drawCommentSurface(screen, 0);
    drawCommentSurface(screen, 1);
    drawMainChain(screen);
}

void Commentator::drawMainChain(Screen* screen) const
{
    // TODO(mayah): assert lock is acquired.

    for (int pi = 0; pi < 2; pi++) {
        if (firingChain_[pi] != nullptr) {
            drawTrace(screen, pi, firingChain_[pi]->trackResult);
        } else if (fireableMainChain_[pi] != nullptr) {
            drawTrace(screen, pi, fireableMainChain_[pi]->trackResult);
        }
    }
}

void Commentator::drawCommentSurface(Screen* screen, int pi) const
{
    // TODO(mayah): assert lock is acquired.

    TTF_Font* font = screen->font();
    if (!font)
        return;

    // What is 7? 656? (656 = right side of main window)
    int LX = 7 + 656 * pi;
    int LX2 = 20 + 656 * pi;
    int LH = 20;

    drawText(screen, "本線", LX, LH * 2);
    if (fireableMainChain_[pi].get()) {
        drawText(screen,
                 to_string(fireableMainChain_[pi]->rensaInfo.chains) + "連鎖" + to_string(fireableMainChain_[pi]->rensaInfo.score) + "点",
                 LX2, LH * 3);
    }
    drawText(screen, "発火可能潰し", LX, LH * 5);
    if (fireableTsubushiChain_[pi].get()) {
        drawText(screen,
                 to_string(fireableTsubushiChain_[pi]->chains()) + "連鎖" +
                 to_string(fireableTsubushiChain_[pi]->score()) + "点",
                 LX2, LH * 6);
        drawText(screen,
                 to_string(fireableTsubushiChain_[pi]->score() / 70) + "個" +
                 to_string(fireableTsubushiChain_[pi]->totalFrames()) + "フレーム",
                 LX2, LH * 7);
    }
    drawText(screen, "発火中/最終発火", LX, LH * 9);
    if (firingChain_[pi].get()) {
        drawText(screen,
                 to_string(firingChain_[pi]->rensaInfo.chains) + "連鎖" +
                 to_string(firingChain_[pi]->rensaInfo.score) + "点",
                 LX2, LH * 10);
    }
    if (!message_[pi].empty())
        drawText(screen, ("AI: " + message_[pi]).c_str(), LX, LH * 22);

    int y = 25;
    for (deque<string>::const_iterator iter = events_[pi].begin(); iter != events_[pi].end(); ++iter) {
        drawText(screen, iter->c_str(), LX, LH * y);
        y++;
    }
}
