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

Commentator::Commentator() :
    shouldStop_(false)
{
    needsUpdate_[0] = needsUpdate_[1] = false;
}

Commentator::~Commentator()
{
    // TODO(mayah): Why don't we have stop() instead of calling pthread_join() here?
    shouldStop_ = true;
    if (th_.joinable())
        th_.join();
}

void Commentator::newGameWillStart()
{
    reset();
}

void Commentator::onUpdate(const GameState& gameState)
{
    lock_guard<mutex> lock(mu_);

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
    th_ = thread([this]() {
        this->runLoop();
    });

    return true;
}

void Commentator::stop()
{
    shouldStop_ = true;
    if (th_.joinable())
        th_.join();
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
                 lock_guard<mutex> lock(mu_);
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
        track->rensaResult = f.simulateAndTrack(&track->trackResult);
        if (track->rensaResult.score > 0) {
            lock_guard<mutex> lock(mu_);
            string msg = std::to_string(track->rensaResult.chains) + "連鎖発火: " + std::to_string(track->rensaResult.score) + "点";
            addEventMessage(pi, msg);
            firingChain_[pi] = move(track);
            fireableMainChain_[pi].reset();
            fireableTsubushiChain_[pi].reset();
            return;
        }

        lock_guard<mutex> lock(mu_);
        firingChain_[pi].reset();
    }

    // 2. Check Tsubushi chain
    {
        KumipuyoSeq kp(kumipuyoSeq);
        if (3 < kp.size())
            kp.resize(3);

        pair<int, double> bestTsubushiScore = make_pair(100, 0.0); // # of hand & score. Smaller is better.
        FeasibleRensaInfo feasibleRensaInfo;
        Plan::iterateAvailablePlans(field, kp, kp.size(), [&bestTsubushiScore, &feasibleRensaInfo](const RefPlan& plan) {
            if (plan.chains() != 2 && plan.chains() != 3)
                return;

            // Considers only >= 2rensa double.
            // 2rensa double = 40 + 80 * (8 + 3) = 40 + 880 = 920
            // 3rensa = 40 + 40 * 8 + 40 * 16 = 1000
            if (plan.score() < 920)
                return;

            pair<int, double> tsubushiScore = make_pair(plan.decisions().size(),
                                                        -static_cast<double>(plan.score()) / plan.totalFrames());
            if (tsubushiScore < bestTsubushiScore) {
                bestTsubushiScore = tsubushiScore;
                feasibleRensaInfo = FeasibleRensaInfo(plan.rensaResult(), plan.initiatingFrames());
            }
        });

        if (bestTsubushiScore.first < 100) {
            lock_guard<mutex> lock(mu_);
            fireableTsubushiChain_[pi].reset(new FeasibleRensaInfo(feasibleRensaInfo));
        }
    }

    // 3. Check Main chain
    {
        vector<TrackedPossibleRensaInfo> rs = RensaDetector::findPossibleRensasWithTracking(field, 3);
        const TrackedPossibleRensaInfo* bestRensa = nullptr;
        int maxScore = 0;
        for (const auto& r : rs) {
            if (maxScore < r.rensaResult.score) {
                maxScore = r.rensaResult.score;
                bestRensa = &r;
            }
        }

        if (bestRensa != nullptr) {
            lock_guard<mutex> lock(mu_);
            fireableMainChain_[pi].reset(new TrackedPossibleRensaInfo(*bestRensa));
        }
    }
}

void Commentator::reset()
{
    lock_guard<mutex> lock(mu_);
    needsUpdate_[0] = needsUpdate_[1] = false;
    message_[0] = message_[1] = "";
    for (int i = 0; i < 2; i++) {
        fireableMainChain_[i].reset();
        fireableTsubushiChain_[i].reset();
        firingChain_[i].reset();
        events_[i].clear();
    }
}

// ----------------------------------------------------------------------

void Commentator::draw(Screen* screen)
{
    lock_guard<mutex> lock(mu_);
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

    // What is 7?
    int LX = 7 + screen->mainBox().dx * pi;
    int LX2 = 20 + screen->mainBox().dx * pi;
    int LH = 20;

    drawText(screen, "本線", LX, LH * 2);
    if (fireableMainChain_[pi].get()) {
        drawText(screen,
                 to_string(fireableMainChain_[pi]->rensaResult.chains) + "連鎖" + to_string(fireableMainChain_[pi]->rensaResult.score) + "点",
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
                 to_string(firingChain_[pi]->rensaResult.chains) + "連鎖" +
                 to_string(firingChain_[pi]->rensaResult.score) + "点",
                 LX2, LH * 10);
    }

    int offsetY = screen->mainBox().dy + 40;
    int y = 0;
    if (!message_[pi].empty())
        drawText(screen, ("AI: " + message_[pi]).c_str(), LX, offsetY + LH * y++);

    for (deque<string>::const_iterator iter = events_[pi].begin(); iter != events_[pi].end(); ++iter) {
        drawText(screen, iter->c_str(), LX, offsetY + LH * y++);
    }
}

void Commentator::addEventMessage(int pi, const string& msg)
{
    // TODO(mayah): Assert locked.
    events_[pi].push_front(msg);
    while (events_[pi].size() > 3)
        events_[pi].pop_back();
}
