#include "core/client/ai/ai.h"

#include <glog/logging.h>

#include <algorithm>

#include "base/base.h"
#include "core/constant.h"
#include "core/core_field.h"
#include "core/decision.h"
#include "core/field_pretty_printer.h"
#include "core/frame_request.h"
#include "core/frame_response.h"
#include "core/kumipuyo.h"
#include "core/plain_field.h"
#include "core/puyo_color.h"
#include "core/rensa_result.h"
#include "core/user_event.h"

using namespace std;

struct DecisionSending {
    void clear()
    {
        *this = DecisionSending();
    }

    DropDecision dropDecision = DropDecision();
    Kumipuyo kumipuyo = Kumipuyo();
    CoreField fieldBeforeThink;

    bool requested = false;
    bool ready = false;
    // True when we need to rethink this hand. This happens when we detected ojama etc.
    bool needsRethink = false;
    bool ojamaDropped = false;
};

AI::AI(int argc, char* argv[], const string& name) :
    AI(name)
{
    UNUSED_VARIABLE(argc);
    UNUSED_VARIABLE(argv);
}

AI::AI(const string& name) :
    name_(name),
    rethinkRequested_(false),
    enemyDecisionRequestFrameId_(0),
    behaviorDefensive_(false),
    behaviorRethinkAfterOpponentRensa_(false)
{
}

AI::~AI()
{
}

// TODO(mayah): Consider to introduce state. It's hard to maintain flags.
// TODO(mayah): ZENKESHI is not accurate enough. For example, when calling think(), the just previous
// decision might erase some puyos. If we had ZENKESHI in that time, ZENKESHI should not be passed to
// think(). However, it does, now.
void AI::runLoop()
{
    DecisionSending next1;

    // nextThinkFrameId is frameId in which the decision of think() is sent.
    int nextThinkFrameId = 0;

    while (true) {
        google::FlushLogFiles(google::INFO);

        FrameRequest frameRequest;
        if (!connector_.receive(&frameRequest)) {
            if (connector_.isClosed()) {
                LOG(INFO) << "connection is closed";
                break;
            }
            LOG(ERROR) << "received unexpected request?";
            break;
        }

        if (!frameRequest.isValid()) {
            connector_.send(FrameResponse(frameRequest.frameId));
            continue;
        }

        if (frameRequest.hasGameEnd()) {
            gameHasEnded(frameRequest);
        }
        // Before starting a new game, we need to think the first hand.
        // TODO(mayah): Maybe game server should send some information that we should initialize.
        if (frameRequest.shouldInitialize()) {
            next1.clear();
            nextThinkFrameId = 0;
            gameWillBegin(frameRequest);
        }

        // Update enemy info if necessary.
        if (frameRequest.enemyPlayerFrameRequest().event.decisionRequest)
            enemyDecisionRequested(frameRequest);
        if (frameRequest.enemyPlayerFrameRequest().event.ojamaDropped)
            enemyOjamaDropped(frameRequest);
        if (frameRequest.enemyPlayerFrameRequest().event.grounded)
            enemyGrounded(frameRequest);
        if (frameRequest.enemyPlayerFrameRequest().event.wnextAppeared)
            enemyNext2Appeared(frameRequest);

        // STATE_YOU_GROUNDED and STATE_WNEXT_APPEARED might come out-of-order.
        bool shouldThink = false;
        if (frameRequest.myPlayerFrameRequest().event.wnextAppeared) {
            next2Appeared(frameRequest);
            shouldThink = true;

            // When hand == 0, nextThinkFrameId will be 0. We'd like to keep frameId is increasing.
            if (nextThinkFrameId < frameRequest.frameId)
                nextThinkFrameId = frameRequest.frameId;
        }
        if (frameRequest.myPlayerFrameRequest().event.puyoErased) {
            shouldThink = true;
            // TODO(mayah): This is not so accurate. We need to consider FRAMES_GROUNDING and frames for dropping.
            nextThinkFrameId = frameRequest.frameId + FRAMES_VANISH_ANIMATION + FRAMES_PREPARING_NEXT;
        }

        if (shouldThink) {
            const auto& kumipuyoSeq = frameRequest.myPlayerFrameRequest().kumipuyoSeq;
            LOG(INFO) << "STATE_WNEXT_APPEARED";
            VLOG(1) << '\n' << me_.field.toDebugString();

            KumipuyoSeq seq = rememberedSequence(me_.hand + 1);
            CHECK_EQ(kumipuyoSeq.get(1), seq.get(0));
            CHECK_EQ(kumipuyoSeq.get(2), seq.get(1));

            next1.fieldBeforeThink = me_.field;
            next1.dropDecision = think(nextThinkFrameId, me_.field, seq,
                                       myPlayerState(), enemyPlayerState(), false);

            next1.kumipuyo = kumipuyoSeq.get(1);
            next1.ready = true;
        }
        // Update my info if necessary.
        if (frameRequest.myPlayerFrameRequest().event.ojamaDropped) {
            // We need to rethink the next1 decision.
            next1.needsRethink = true;
            next1.ojamaDropped = true;
            ojamaDropped(frameRequest);
        }
        if (frameRequest.myPlayerFrameRequest().event.grounded) {
            grounded(frameRequest);
        }
        if (frameRequest.myPlayerFrameRequest().event.decisionRequest) {
            VLOG(1) << "REQUESTED";
            next1.requested = true;
            decisionRequested(frameRequest);
        }
        if (frameRequest.myPlayerFrameRequest().event.decisionRequestAgain) {
            // We need to handle this specially. Since we've proceeded next1, we don't have any knowledge about this turn.
            // TODO(mayah): Should we preserve DecisionSending after we used it for this?
            VLOG(1) << "REQUEST_AGAIN";
            DCHECK(!frameRequest.myPlayerFrameRequest().event.decisionRequest)
                << "decisionRequestAgain should not come with decisionRequest.";
            DropDecision dropDecision = think(frameRequest.frameId,
                                              CoreField(frameRequest.myPlayerFrameRequest().field),
                                              frameRequest.myPlayerFrameRequest().kumipuyoSeq,
                                              myPlayerState(),
                                              enemyPlayerState(),
                                              true);
            connector_.send(FrameResponse(frameRequest.frameId, dropDecision.decision(), dropDecision.message()));
            continue;
        }

        if (!next1.requested || !next1.ready) {
            connector_.send(FrameResponse(frameRequest.frameId));
            continue;
        }

        // Check field inconsistency. We only check when me_hand >= 3, since we cannot trust the field in 3 hands.
        if (me_.hand >= 3 && isFieldInconsistent(next1.fieldBeforeThink, frameRequest.myPlayerFrameRequest().field)) {
            LOG(INFO) << "FIELD INCONSISTENCY DETECTED: hand=" << me_.hand;
            VLOG(1) << '\n' << FieldPrettyPrinter::toStringFromMultipleFields(
                { next1.fieldBeforeThink, frameRequest.myPlayerFrameRequest().field },
                { frameRequest.myPlayerFrameRequest().kumipuyoSeq, frameRequest.myPlayerFrameRequest().kumipuyoSeq });

            next1.needsRethink = true;
        }

        // Rethink if necessary.
        if (next1.needsRethink || rethinkRequested_) {
            LOG(INFO) << "RETHINK";

            mergeField(&me_.field, frameRequest.myPlayerFrameRequest().field, next1.ojamaDropped);
            const auto& kumipuyoSeq = frameRequest.myPlayerFrameRequest().kumipuyoSeq;

            KumipuyoSeq seq = rememberedSequence(me_.hand);
            CHECK_EQ(kumipuyoSeq.get(0), seq.get(0));
            CHECK_EQ(kumipuyoSeq.get(1), seq.get(1));

            next1.dropDecision = think(frameRequest.frameId, me_.field, seq, myPlayerState(), enemyPlayerState(), true);
            next1.kumipuyo = kumipuyoSeq.get(0);
            next1.ready = true;
            next1.needsRethink = false;
            next1.ojamaDropped = false;
            rethinkRequested_ = false;
        }

        // Send
        connector_.send(FrameResponse(frameRequest.frameId, next1.dropDecision.decision(), next1.dropDecision.message()));
        nextThinkFrameId =
            frameRequest.frameId +
            next1.fieldBeforeThink.framesToDropNext(next1.dropDecision.decision()) +
            FRAMES_PREPARING_NEXT;

        // Move to next.
        if (next1.dropDecision.decision().isValid() && next1.kumipuyo.isValid()) {
            if (!me_.field.dropKumipuyo(next1.dropDecision.decision(), next1.kumipuyo)) {
                LOG(WARNING) << "failed to drop kumipuyo. Moving to impossible position?";
            }
            me_.field.simulate();
        }
        next1.clear();
    }

    LOG(INFO) << "will exit run loop";
}

void AI::gaze(int frameId, const CoreField&, const KumipuyoSeq&)
{
    UNUSED_VARIABLE(frameId);
}

void AI::gameWillBegin(const FrameRequest& frameRequest)
{
    me_.clear();
    enemy_.clear();

    rethinkRequested_ = false;
    enemyDecisionRequestFrameId_ = 0;

    onGameWillBegin(frameRequest);
}

void AI::gameHasEnded(const FrameRequest& frameRequest)
{
    onGameHasEnded(frameRequest);
}

void AI::decisionRequested(const FrameRequest& frameRequest)
{
    me_.hand += 1;

    // our field will be updated in runLoop().

    if (enemy_.pendingOjama > 0) {
        enemy_.fixedOjama += enemy_.pendingOjama;
        enemy_.pendingOjama = 0;
    }

    me_.isRensaOngoing = false;
    me_.finishingRensaFrameId = 0;
    me_.ongoingRensaResult = RensaResult();

    if (!me_.hasOjamaDropped)
        me_.fixedOjama = 0;
    me_.hasOjamaDropped = false;

    onDecisionRequested(frameRequest);
}

void AI::grounded(const FrameRequest& frameRequest)
{
    CoreField field(frameRequest.myPlayerFrameRequest().field);
    field.forceDrop();
    RensaResult rensaResult = field.simulate();
    if (rensaResult.chains > 0) {
        int ojamaCount = rensaResult.score / 70;;
        if (me_.hasZenkeshi) {
            ojamaCount += 30;
        }

        if (me_.pendingOjama > 0) {
            if (me_.pendingOjama > ojamaCount) {
                me_.pendingOjama -= ojamaCount;
                ojamaCount = 0;
            } else {
                ojamaCount -= me_.pendingOjama;
                me_.pendingOjama = 0;
            }
        }
        if (me_.fixedOjama > 0) {
            if (me_.fixedOjama > ojamaCount) {
                me_.fixedOjama -= ojamaCount;
                ojamaCount = 0;
            } else {
                ojamaCount -= me_.fixedOjama;
                me_.fixedOjama = 0;
            }
        }

        enemy_.pendingOjama += ojamaCount;

        me_.isRensaOngoing = true;
        me_.ongoingRensaResult = rensaResult;
        me_.finishingRensaFrameId = frameRequest.frameId + rensaResult.frames;
        me_.hasZenkeshi = false;
    }

    // Don't check zenkeshi if me_.hand is 0.
    if (me_.hand != 0 && field.isZenkeshi()) {
        me_.hasZenkeshi = true;
    }

    onGrounded(frameRequest);
}

void AI::ojamaDropped(const FrameRequest& frameRequest)
{
    me_.fixedOjama = std::max(me_.fixedOjama - 30, 0);
    me_.hasOjamaDropped = true;
    onOjamaDropped(frameRequest);
}

void AI::next2Appeared(const FrameRequest& frameRequest)
{
    const KumipuyoSeq& kumipuyoSeq = frameRequest.myPlayerFrameRequest().kumipuyoSeq;

    for (int i = 0; i < 3; ++i) {
        if (me_.hand + i < me_.seq.size()) {
            CHECK_EQ(me_.seq.get(me_.hand + i), kumipuyoSeq.get(i))
                << me_.hand << " " << i << " " << me_.seq.size()
                << me_.seq.toString() << " " << kumipuyoSeq.toString();
        } else {
            me_.seq.add(kumipuyoSeq.get(i));
        }
    }

    onNext2Appeared(frameRequest);
}

void AI::enemyDecisionRequested(const FrameRequest& frameRequest)
{
    enemyDecisionRequestFrameId_ = frameRequest.frameId;

    enemy_.hand += 1;
    enemy_.field = CoreField(frameRequest.enemyPlayerFrameRequest().field);
    enemy_.field.forceDrop();

    // Should this be here?
    if (me_.pendingOjama > 0) {
        me_.fixedOjama += me_.pendingOjama;
        me_.pendingOjama = 0;
    }
    enemy_.isRensaOngoing = false;
    enemy_.finishingRensaFrameId = 0;
    enemy_.ongoingRensaResult = RensaResult();

    if (!enemy_.hasOjamaDropped)
        enemy_.fixedOjama = 0;
    enemy_.hasOjamaDropped = false;

    onEnemyDecisionRequested(frameRequest);
}

void AI::enemyGrounded(const FrameRequest& frameRequest)
{
    CoreField field(frameRequest.enemyPlayerFrameRequest().field);
    field.forceDrop();

    RensaResult rensaResult = field.simulate();

    if (rensaResult.chains > 0) {
        LOG(INFO) << "Detected the opponent has fired rensa: " << rensaResult.toString();
        if (behaviorRethinkAfterOpponentRensa_)
            requestRethink();

        int ojamaCount = rensaResult.score / 70;;
        if (enemy_.hasZenkeshi) {
            ojamaCount += 30;
        }

        if (enemy_.pendingOjama > 0) {
            if (enemy_.pendingOjama > ojamaCount) {
                enemy_.pendingOjama -= ojamaCount;
                ojamaCount = 0;
            } else {
                ojamaCount -= enemy_.pendingOjama;
                enemy_.pendingOjama = 0;
            }
        }
        if (enemy_.fixedOjama > 0) {
            if (enemy_.fixedOjama > ojamaCount) {
                enemy_.fixedOjama -= ojamaCount;
                ojamaCount = 0;
            } else {
                ojamaCount -= enemy_.fixedOjama;
                enemy_.fixedOjama = 0;
            }
        }

        me_.pendingOjama = ojamaCount;

        enemy_.isRensaOngoing = true;
        enemy_.ongoingRensaResult = rensaResult;
        enemy_.finishingRensaFrameId = frameRequest.frameId + rensaResult.frames;
        enemy_.hasZenkeshi = false;
    }

    if (enemy_.hand != 0 && field.isZenkeshi()) {
        enemy_.hasZenkeshi = true;
    }

    onEnemyGrounded(frameRequest);
}

void AI::enemyOjamaDropped(const FrameRequest& frameRequest)
{
    enemy_.fixedOjama = std::max(enemy_.fixedOjama - 30, 0);
    enemy_.hasOjamaDropped = true;
    onEnemyOjamaDropped(frameRequest);
}

void AI::enemyNext2Appeared(const FrameRequest& frameRequest)
{
    const KumipuyoSeq& kumipuyoSeq = frameRequest.enemyPlayerFrameRequest().kumipuyoSeq;

    for (int i = 0; i < 3; ++i) {
        if (enemy_.hand + i < enemy_.seq.size()) {
            CHECK_EQ(enemy_.seq.get(enemy_.hand + i), kumipuyoSeq.get(i));
        } else {
            enemy_.seq.add(kumipuyoSeq.get(i));
        }
    }

    // When enemy_.hand == 0, rememberedSequence(0) contains PuyoColor::EMPTY.
    // So, don't gaze at that time.
    if (enemy_.hand > 0)
        gaze(enemyDecisionRequestFrameId_, enemy_.field, rememberedSequence(enemy_.hand));

    onEnemyNext2Appeared(frameRequest);
}

// static
bool AI::isFieldInconsistent(const PlainField& f, const PlainField& provided)
{
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            if (f.color(x, y) != provided.color(x, y))
                return true;
        }
    }

    return false;
}

// static
void AI::mergeField(CoreField* f, const PlainField& provided, bool ojamaDropped)
{
    bool unexpectedMismatch = false;
    int possibleMinOjamaHeight = 0;
    int possibleMaxOjamaHeight = 5;
    int ojamaHeights[FieldConstant::MAP_WIDTH] {};

    for (int x = 1; x <= 6; ++x) {
        bool restIsEmpty = false;
        bool ojama12th = false;
        int ojamaHeight = 0;
        int y;
        for (y = 1; y <= 12; ++y) {
            if (f->color(x, y) == provided.color(x, y))
                continue;
            if (f->color(x, y) == PuyoColor::EMPTY && provided.color(x, y) == PuyoColor::OJAMA) {
                ++ojamaHeight;
                if (y == 12)
                    ojama12th = true;
                f->unsafeSet(x, y, provided.color(x, y));
                continue;
            }

            // Here, detected non-ojama mismatch.
            unexpectedMismatch = true;
            if (provided.color(x, y) == PuyoColor::EMPTY) {
                restIsEmpty = true;
                break;
            }
            f->unsafeSet(x, y, provided.color(x, y));
        }

        ojamaHeights[x] = ojamaHeight;
        if (ojama12th) {
            possibleMinOjamaHeight = std::max(ojamaHeight, possibleMinOjamaHeight);
        } else {
            possibleMinOjamaHeight = std::max(ojamaHeight, possibleMinOjamaHeight);
            possibleMaxOjamaHeight = std::min(ojamaHeight + 1, possibleMaxOjamaHeight);
        }

        if (restIsEmpty) {
            for (; y <= 13; ++y) {
                DCHECK_EQ(PuyoColor::EMPTY, provided.color(x, y));
                f->unsafeSet(x, y, PuyoColor::EMPTY);
            }
        }
        f->recalcHeightOn(x);
    }

    // TODO(mayah): Is this right? Not 100% sure...
    int possibleOjamaHeight = 5;
    if (!unexpectedMismatch) {
        if (possibleMinOjamaHeight == possibleMaxOjamaHeight || possibleMinOjamaHeight + 1 == possibleMaxOjamaHeight) {
            possibleOjamaHeight = possibleMaxOjamaHeight;
        } else {
            possibleOjamaHeight = possibleMaxOjamaHeight;
        }
    }

    if (ojamaDropped) {
        for (int x = 1; x <= 6; ++x) {
            if (ojamaHeights[x] < possibleOjamaHeight && f->color(x, 12) == PuyoColor::OJAMA) {
                f->dropPuyoOn(x, PuyoColor::OJAMA);
            }
        }
    }
}

KumipuyoSeq AI::rememberedSequence(int indexFrom) const
{
    if (me_.seq.size() < enemy_.seq.size())
        return enemy_.seq.subsequence(indexFrom);
    else
        return me_.seq.subsequence(indexFrom);
}
