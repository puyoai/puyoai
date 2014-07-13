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

CommentatorResult Commentator::result() const
{
    lock_guard<mutex> lock(mu_);
    CommentatorResult r;
    for (int pi = 0; pi < 2; ++pi) {
        if (fireableMainChain_[pi].get()) {
            r.fireableMainChain[pi] = *fireableMainChain_[pi];
        }
        if (fireableTsubushiChain_[pi].get()) {
            r.fireableTsubushiChain[pi] = *fireableTsubushiChain_[pi];
        }
        if (firingChain_[pi].get()) {
            r.firingChain[pi] = *firingChain_[pi];
        }
        r.message[pi] = message_[pi];
        r.events[pi] = events_[pi];
    }
    return r;
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
        int bestScore = 0;
        unique_ptr<TrackedPossibleRensaInfo> bestRensa;
        auto callback = [&](const CoreField&, const RensaResult& rensaResult,
                            const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos,
                            const RensaTrackResult& trackResult) {
            if (bestScore < rensaResult.score) {
                bestScore = rensaResult.score;
                bestRensa.reset(new TrackedPossibleRensaInfo(rensaResult, keyPuyos, firePuyos, trackResult));
            }
        };

        RensaDetector::iteratePossibleRensasWithTracking(field, 3, callback, RensaDetector::Mode::FLOAT);

        if (bestRensa != nullptr) {
            lock_guard<mutex> lock(mu_);
            fireableMainChain_[pi] = move(bestRensa);
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

void Commentator::addEventMessage(int pi, const string& msg)
{
    // TODO(mayah): Assert locked.
    events_[pi].push_front(msg);
    while (events_[pi].size() > 3)
        events_[pi].pop_back();
}

// ----------------------------------------------------------------------

CommentatorDrawer::CommentatorDrawer(const Commentator* commentator) :
    commentator_(commentator)
{
}

CommentatorDrawer::~CommentatorDrawer()
{
}

void CommentatorDrawer::draw(Screen* screen)
{
    CommentatorResult result = commentator_->result();
    drawCommentSurface(screen, result, 0);
    drawCommentSurface(screen, result, 1);
    drawMainChain(screen, result);
}

void CommentatorDrawer::drawMainChain(Screen* screen, const CommentatorResult& result) const
{
    for (int pi = 0; pi < 2; pi++) {
        if (result.firingChain[pi].chains() > 0) {
            drawTrace(screen, pi, result.firingChain[pi].trackResult);
            continue;
        }

        if (result.fireableMainChain[pi].chains() > 0) {
            drawTrace(screen, pi, result.fireableMainChain[pi].trackResult);
            continue;
        }
    }
}

void CommentatorDrawer::drawCommentSurface(Screen* screen, const CommentatorResult& result, int pi) const
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
    if (result.fireableMainChain[pi].chains() > 0) {
        int chains = result.fireableMainChain[pi].chains();
        int score = result.fireableMainChain[pi].score();
        drawText(screen,
                 to_string(chains) + "連鎖" + to_string(score) + "点",
                 LX2, LH * 3);
    }
    drawText(screen, "発火可能潰し", LX, LH * 5);
    if (result.fireableTsubushiChain[pi].chains() > 0) {
        int chains = result.fireableTsubushiChain[pi].chains();
        int score = result.fireableTsubushiChain[pi].score();
        int frames = result.fireableTsubushiChain[pi].totalFrames();
        drawText(screen,
                 to_string(chains) + "連鎖" + to_string(score) + "点",
                 LX2, LH * 6);
        drawText(screen,
                 to_string(score / 70) + "個" + to_string(frames) + "フレーム",
                 LX2, LH * 7);
    }
    drawText(screen, "発火中/最終発火", LX, LH * 9);
    if (result.firingChain[pi].chains() > 0) {
        int chains = result.firingChain[pi].chains();
        int score = result.firingChain[pi].score();
        drawText(screen,
                 to_string(chains) + "連鎖" + to_string(score) + "点",
                 LX2, LH * 10);
    }

    int offsetY = screen->mainBox().dy + 40;
    int y = 0;
    if (!result.message[pi].empty())
        drawText(screen, ("AI: " + result.message[pi]).c_str(), LX, offsetY + LH * y++);

    for (const auto& msg : result.events[pi]) {
        drawText(screen, msg.c_str(), LX, offsetY + LH * y++);
    }
}
