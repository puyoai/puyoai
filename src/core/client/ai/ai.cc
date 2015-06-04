#include "core/client/ai/ai.h"

#include <glog/logging.h>

#include <algorithm>

#include "base/base.h"
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
    desynced_(false),
    rethinkRequested_(false),
    enemyDecisionRequestFrameId_(0),
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
            decisionRequestedForEnemy(frameRequest);
        if (frameRequest.enemyPlayerFrameRequest().event.ojamaDropped)
            ojamaDroppedForEnemy(frameRequest);
        if (frameRequest.enemyPlayerFrameRequest().event.grounded)
            groundedForEnemy(frameRequest);
        if (frameRequest.enemyPlayerFrameRequest().event.puyoErased)
            puyoErasedForEnemy(frameRequest);
        if (frameRequest.enemyPlayerFrameRequest().event.wnextAppeared)
            next2AppearedForEnemy(frameRequest);

        // STATE_YOU_GROUNDED and STATE_WNEXT_APPEARED might come out-of-order.
        bool shouldThink = false;
        if (frameRequest.myPlayerFrameRequest().event.wnextAppeared) {
            next2AppearedForMe(frameRequest);
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

            KumipuyoSeq seq = rememberedSequence(me_.hand + 1, kumipuyoSeq.subsequence(1));
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
            ojamaDroppedForMe(frameRequest);
        }
        if (frameRequest.myPlayerFrameRequest().event.grounded)
            groundedForMe(frameRequest);
        if (frameRequest.myPlayerFrameRequest().event.puyoErased)
            puyoErasedForMe(frameRequest);
        if (frameRequest.myPlayerFrameRequest().event.preDecisionRequest)
            preDecisionRequestedForMe(frameRequest);
        if (frameRequest.myPlayerFrameRequest().event.decisionRequest) {
            VLOG(1) << "REQUESTED";
            next1.requested = true;
            decisionRequestedForMe(frameRequest);
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
            FrameResponse resp(frameRequest.frameId);

            const bool needsSendPreDecision = next1.ready &&
                next1.dropDecision.decision().isValid() &&
                frameRequest.myPlayerFrameRequest().event.preDecisionRequest;
            // Sends pre decision.
            if (needsSendPreDecision) {
                resp.preDecision = next1.dropDecision.decision();
            }

            connector_.send(resp);
            continue;
        }

        // Check field inconsistency. We only check when me_hand >= 3, since we cannot trust the field in 3 hands.
        if (me_.hand >= 3 && isFieldInconsistent(next1.fieldBeforeThink.toPlainField(), frameRequest.myPlayerFrameRequest().field)) {
            LOG(INFO) << "FIELD INCONSISTENCY DETECTED: hand=" << me_.hand;
            VLOG(1) << '\n' << FieldPrettyPrinter::toStringFromMultipleFields(
                { next1.fieldBeforeThink.toPlainField(), frameRequest.myPlayerFrameRequest().field },
                { frameRequest.myPlayerFrameRequest().kumipuyoSeq, frameRequest.myPlayerFrameRequest().kumipuyoSeq });

            next1.needsRethink = true;
        }

        // Rethink if necessary.
        if (next1.needsRethink || rethinkRequested_) {
            LOG(INFO) << "RETHINK";

            me_.field = mergeField(me_.field, frameRequest.myPlayerFrameRequest().field, next1.ojamaDropped);
            const auto& kumipuyoSeq = frameRequest.myPlayerFrameRequest().kumipuyoSeq;

            KumipuyoSeq seq = rememberedSequence(me_.hand, kumipuyoSeq);
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

    desynced_ = false;

    rethinkRequested_ = false;
    enemyDecisionRequestFrameId_ = 0;

    onGameWillBegin(frameRequest);
}

void AI::gameHasEnded(const FrameRequest& frameRequest)
{
    onGameHasEnded(frameRequest);
}

void AI::preDecisionRequestedForMe(const FrameRequest& frameRequest)
{
    // Do nothing.
    onPreDecisionRequestedForMe(frameRequest);
}

void AI::decisionRequestedForMe(const FrameRequest& frameRequest)
{
    // Don't update me_.field here. Our field will be updated in runLoop().
    me_.hand += 1;

    decisionRequestedForCommon(&me_, &enemy_);
    onDecisionRequestedForMe(frameRequest);
}

void AI::decisionRequestedForEnemy(const FrameRequest& frameRequest)
{
    enemyDecisionRequestFrameId_ = frameRequest.frameId;
    enemy_.hand += 1;
    enemy_.field = CoreField::fromPlainFieldWithDrop(frameRequest.enemyPlayerFrameRequest().field);

    decisionRequestedForCommon(&enemy_, &me_);
    onDecisionRequestedForEnemy(frameRequest);
}

// static
void AI::decisionRequestedForCommon(PlayerState* p1, PlayerState* p2)
{
    // TODO(mayah): pending ojama should be committed when rensa is finished.
    if (p2->pendingOjama > 0) {
        p2->fixedOjama += p2->pendingOjama;
        p2->pendingOjama = 0;
    }

    p1->currentChain = 0;
    p1->currentChainStartedFrameId = 0;
    p1->currentRensaResult = RensaResult();

    if (!p1->hasOjamaDropped)
        p1->fixedOjama = 0;
    p1->hasOjamaDropped = false;
}

void AI::groundedForMe(const FrameRequest& frameRequest)
{
    // Should we take field from me_.field? It's more accurate than frameRequest?
    me_.fieldWhenGrounded = CoreField::fromPlainFieldWithDrop(frameRequest.myPlayerFrameRequest().field);
    groundedForCommon(&me_, frameRequest.frameId);
    onGroundedForMe(frameRequest);
}

void AI::groundedForEnemy(const FrameRequest& frameRequest)
{
    enemy_.fieldWhenGrounded = CoreField::fromPlainFieldWithDrop(frameRequest.enemyPlayerFrameRequest().field);
    groundedForCommon(&enemy_, frameRequest.frameId);
    onGroundedForEnemy(frameRequest);
}

void AI::groundedForCommon(PlayerState* state, int frameId)
{
    CoreField cf(state->fieldWhenGrounded);
    VLOG(1) << cf;
    state->currentRensaResult = cf.simulate();

    if (state->currentRensaResult.chains > 0) {
        state->currentChainStartedFrameId = frameId;
        // Consider ZENKESHI is consumed here.
        state->hasZenkeshi = false;
    } else {
        state->currentChainStartedFrameId = 0;
    }

    // We check ZENKESHI here, so that we can notice ZENKESHI earlier.
    if (state->hand != 0 && cf.isZenkeshi()) {
        state->hasZenkeshi = true;
    }
}

void AI::puyoErasedForMe(const FrameRequest& frameRequest)
{
    puyoErasedForCommon(&me_, &enemy_, frameRequest.frameId, frameRequest.myPlayerFrameRequest().field);
    onPuyoErasedForMe(frameRequest);
}

void AI::puyoErasedForEnemy(const FrameRequest& frameRequest)
{
    puyoErasedForCommon(&enemy_, &me_, frameRequest.frameId, frameRequest.enemyPlayerFrameRequest().field);
    onPuyoErasedForEnemy(frameRequest);
}

// static
void AI::puyoErasedForCommon(PlayerState* p1, PlayerState* p2, int frameId, const PlainField& provided)
{
    p1->currentChain += 1;
    p1->currentChainStartedFrameId = frameId;

    CoreField cf(CoreField::fromPlainFieldWithDrop(provided));
    CoreField::SimulationContext context(p1->currentChain);
    RensaStepResult stepResult = cf.vanishDrop(&context);

    p1->unusedScore += stepResult.score;
    int ojamaCount = p1->unusedScore / 70;
    p1->unusedScore %= 70;

    if (p1->pendingOjama > 0) {
        if (p1->pendingOjama > ojamaCount) {
            p1->pendingOjama -= ojamaCount;
            ojamaCount = 0;
        } else {
            ojamaCount -= p1->pendingOjama;
            p1->pendingOjama = 0;
        }
    }
    if (p1->fixedOjama > 0) {
        if (p1->fixedOjama > ojamaCount) {
            p1->fixedOjama -= ojamaCount;
            ojamaCount = 0;
        } else {
            ojamaCount -= p1->fixedOjama;
            p1->fixedOjama = 0;
        }
    }

    p2->pendingOjama += ojamaCount;

    if (cf.isZenkeshi()) {
        // p1->hasZenkeshi is set in puyoErasedOnCommon.
        p1->unusedScore += scoreForOjama(30);
    }

    // Estimate the rest of chains.
    cf = CoreField::fromPlainFieldWithDrop(provided);
    context = CoreField::SimulationContext(p1->currentChain);
    p1->currentRensaResult = cf.simulate(&context);
    // It's already used. We don't want to count it again.
    p1->currentRensaResult.score -= stepResult.score;
}

void AI::ojamaDroppedForMe(const FrameRequest& frameRequest)
{
    ojamaDroppedForCommon(&me_);
    onOjamaDroppedForMe(frameRequest);
}

void AI::ojamaDroppedForEnemy(const FrameRequest& frameRequest)
{
    ojamaDroppedForCommon(&enemy_);
    onOjamaDroppedForEnemy(frameRequest);
}

// static
void AI::ojamaDroppedForCommon(PlayerState* state)
{
    state->fixedOjama = std::max(state->fixedOjama - 30, 0);
    state->hasOjamaDropped = true;
}

void AI::next2AppearedForMe(const FrameRequest& frameRequest)
{
    const KumipuyoSeq& kumipuyoSeq = frameRequest.myPlayerFrameRequest().kumipuyoSeq;
    next2AppearedForCommon(&me_, kumipuyoSeq);
    onNext2AppearedForMe(frameRequest);
}

void AI::next2AppearedForEnemy(const FrameRequest& frameRequest)
{
    const KumipuyoSeq& kumipuyoSeq = frameRequest.enemyPlayerFrameRequest().kumipuyoSeq;
    next2AppearedForCommon(&enemy_, kumipuyoSeq);

    // When enemy_.hand == 0, rememberedSequence(0) contains PuyoColor::EMPTY.
    // So, don't gaze at that time.
    if (enemy_.hand > 0)
        gaze(enemyDecisionRequestFrameId_, enemy_.field, rememberedSequence(enemy_.hand, kumipuyoSeq));

    onNext2AppearedForEnemy(frameRequest);
}

void AI::next2AppearedForCommon(PlayerState* state, const KumipuyoSeq& kumipuyoSeq)
{
    if (desynced_)
        return;

    for (int i = 0; i < 3; ++i) {
        if (state->hand + i < state->seq.size()) {
            if (state->seq.get(state->hand + i) != kumipuyoSeq.get(i))
                desynced_ = true;
        } else {
            state->seq.add(kumipuyoSeq.get(i));
        }
    }
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
CoreField AI::mergeField(const CoreField& cf, const PlainField& provided, bool ojamaDropped)
{
    CoreField result;
    bool unexpectedMismatch = false;
    int possibleMinOjamaHeight = 0;
    int possibleMaxOjamaHeight = 5;
    int ojamaHeights[FieldConstant::MAP_WIDTH] {};

    for (int x = 1; x <= 6; ++x) {
        bool ojama12th = false;
        int ojamaHeight = 0;
        for (int y = 1; y <= 12; ++y) {
            if (provided.isEmpty(x, y))
                break;

            if (cf.color(x, y) == provided.color(x, y)) {
                result.dropPuyoOn(x, cf.color(x, y));
            } else if (cf.color(x, y) == PuyoColor::EMPTY && provided.color(x, y) == PuyoColor::OJAMA) {
                ++ojamaHeight;
                if (y == 12)
                    ojama12th = true;
                result.dropPuyoOn(x, PuyoColor::OJAMA);
            } else {
                // Here, detected non-ojama mismatch.
                unexpectedMismatch = true;
                result.dropPuyoOn(x, provided.color(x, y));
            }
        }

        // Check 13th row.
        if (!result.isEmpty(x, 12) && !cf.isEmpty(x, 13)) {
            result.dropPuyoOn(x, cf.color(x, 13));
        }

        ojamaHeights[x] = ojamaHeight;
        if (ojama12th) {
            possibleMinOjamaHeight = std::max(ojamaHeight, possibleMinOjamaHeight);
        } else {
            possibleMinOjamaHeight = std::max(ojamaHeight, possibleMinOjamaHeight);
            possibleMaxOjamaHeight = std::min(ojamaHeight + 1, possibleMaxOjamaHeight);
        }
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
            if (ojamaHeights[x] < possibleOjamaHeight && result.color(x, 12) == PuyoColor::OJAMA) {
                result.dropPuyoOn(x, PuyoColor::OJAMA);
            }
        }
    }

    return result;
}

KumipuyoSeq AI::rememberedSequence(int indexFrom, const KumipuyoSeq& provided) const
{
    if (desynced_)
        return provided;

    if (me_.seq.size() < enemy_.seq.size())
        return enemy_.seq.subsequence(indexFrom);
    else
        return me_.seq.subsequence(indexFrom);
}
