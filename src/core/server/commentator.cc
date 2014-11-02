// -*- encoding: utf-8 -*-

#include "core/server/commentator.h"

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include <glog/logging.h>

#include "base/base.h"
#include "core/algorithm/plan.h"
#include "core/algorithm/rensa_detector.h"
#include "core/server/game_state.h"
#include "core/state.h"

using namespace std;

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

void Commentator::addCommentatorObserver(CommentatorObserver* observer)
{
    CHECK(observer);
    CHECK(!hasStarted_);

    observers_.push_back(observer);
}

void Commentator::newGameWillStart()
{
    reset();
}

void Commentator::onUpdate(const GameState& gameState)
{
    lock_guard<mutex> lock(mu_);

    for (int i = 0; i < 2; ++i) {
        const PlayerGameState& pgs = gameState.playerGameState(i);
        if (pgs.state.grounded) {
            frameId_[i] = gameState.frameId();
            field_[i] = CoreField(pgs.field);
            // When chigiri is used, some puyo exists in the air. So we need to drop.
            field_[i].forceDrop();
            kumipuyoSeq_[i] = pgs.kumipuyoSeq;
            needsUpdate_[i] = true;
        }

        if (!pgs.message.empty())
            message_[i] = pgs.message;
    }
}

CommentatorResult Commentator::result() const
{
    lock_guard<mutex> lock(mu_);
    CommentatorResult r;
    for (int pi = 0; pi < 2; ++pi) {
        r.frameId[pi] = frameId_[pi];
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

    hasStarted_ = true;
    return true;
}

void Commentator::stop()
{
    shouldStop_ = true;
    if (th_.joinable()) {
        th_.join();
    }

    hasStarted_ = false;
}

void Commentator::runLoop()
{
    while (!shouldStop_) {
        struct timespec req { 0, 16 * 1000 * 1000 };
        struct timespec rem;
        while (nanosleep(&req, &rem) < 0) {
            PCHECK(errno == EINTR);
            req = rem;
        }

        bool updated = false;
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
            updated = true;
        }

        if (updated) {
            CommentatorResult r = result();
            for (auto observer : observers_) {
                observer->onCommentatorResultUpdate(r);
            }
        }
    }
}

void Commentator::update(int pi, const CoreField& field, const KumipuyoSeq& kumipuyoSeq)
{
    for (const Kumipuyo& kp : kumipuyoSeq) {
        if (kp.axis == PuyoColor::EMPTY || kp.child == PuyoColor::EMPTY)
            return;
    }

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
                feasibleRensaInfo = FeasibleRensaInfo(plan.rensaResult(), plan.framesToInitiate());
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
