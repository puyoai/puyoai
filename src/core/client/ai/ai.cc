#include "core/client/ai/ai.h"

#include "core/field/rensa_result.h"
#include "core/field_pretty_printer.h"
#include "core/frame_request.h"
#include "core/frame_response.h"
#include "core/state.h"

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
};

void AdditionalThoughtInfo::setOngoingRensa(const RensaResult& rensaResult, int finishingFrameId)
{
    isRensaOngoing_ = true;
    ongoingRensaResult_ = rensaResult;
    finishingRensaFrameId_ = finishingFrameId;
}

void AdditionalThoughtInfo::unsetOngoingRensa()
{
    isRensaOngoing_ = false;
}

AI::AI(int argc, char* argv[], const string& name) :
    name_(name),
    hand_(0),
    enemyHand_(0),
    rethinkRequested_(false),
    enemyDecisionRequestFrameId_(0),
    behaviorDefensive_(false),
    behaviorRethinkAfterOpponentRensa_(false)
{
    UNUSED_VARIABLE(argc);
    UNUSED_VARIABLE(argv);
}

AI::AI(const string& name) :
    name_(name),
    hand_(0),
    enemyHand_(0),
    rethinkRequested_(false),
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

    while (true) {
        google::FlushLogFiles(google::INFO);

        FrameRequest frameRequest = connector_.receive();
        if (frameRequest.connectionLost) {
            LOG(INFO) << "connection lost";
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
        if (frameRequest.frameId == 1) {
            next1.clear();
            gameWillBegin(frameRequest);
        }

        // Update enemy info if necessary.
        if (frameRequest.enemyPlayerFrameRequest().state.decisionRequest)
            enemyDecisionRequested(frameRequest);
        if (frameRequest.enemyPlayerFrameRequest().state.grounded) {
            enemyGrounded(frameRequest);
        }
        if (frameRequest.enemyPlayerFrameRequest().state.wnextAppeared)
            enemyNext2Appeared(frameRequest);

        // STATE_YOU_GROUNDED and STATE_WNEXT_APPEARED might come out-of-order.
        if (frameRequest.myPlayerFrameRequest().state.wnextAppeared) {
            next2Appeared(frameRequest);
            const auto& kumipuyoSeq = frameRequest.myPlayerFrameRequest().kumipuyoSeq;
            LOG(INFO) << "STATE_WNEXT_APPEARED";
            VLOG(1) << '\n' << field_.toDebugString();

            KumipuyoSeq seq = rememberedSequence(hand_ + 1);
            CHECK_EQ(kumipuyoSeq.get(1), seq.get(0));
            CHECK_EQ(kumipuyoSeq.get(2), seq.get(1));

            next1.fieldBeforeThink = field_;
            next1.dropDecision = think(frameRequest.frameId, field_, seq, additionalThoughtInfo_, false);

            next1.kumipuyo = kumipuyoSeq.get(1);
            next1.ready = true;
        }
        if (frameRequest.myPlayerFrameRequest().state.grounded) {
            grounded(frameRequest);
        }

        // Update my info if necessary.
        if (frameRequest.myPlayerFrameRequest().state.ojamaDropped) {
            // We need to rethink the next1 decision.
            next1.needsRethink = true;
        }
        if (frameRequest.myPlayerFrameRequest().state.decisionRequest) {
            VLOG(1) << "REQUESTED";
            next1.requested = true;
            decisionRequested(frameRequest);
        }
        if (frameRequest.myPlayerFrameRequest().state.decisionRequestAgain) {
            // We need to handle this specially. Since we've proceeded next1, we don't have any knowledge about this turn.
            // TODO(mayah): Should we preserve DecisionSending after we used it for this?
            VLOG(1) << "REQUEST_AGAIN";
            DCHECK(!frameRequest.myPlayerFrameRequest().state.decisionRequest) << "decisionRequestAgain should not come with decisionRequest.";
            DropDecision dropDecision = think(frameRequest.frameId,
                                              frameRequest.myPlayerFrameRequest().field,
                                              frameRequest.myPlayerFrameRequest().kumipuyoSeq,
                                              additionalThoughtInfo_, true);
            connector_.send(FrameResponse(frameRequest.frameId, dropDecision.decision(), dropDecision.message()));
            continue;
        }

        if (!next1.requested || !next1.ready) {
            connector_.send(FrameResponse(frameRequest.frameId));
            continue;
        }

        // Check field inconsistency. We only check when hand_ >= 3, since we cannot trust the field in 3 hands.
        if (hand_ >= 3 && isFieldInconsistent(next1.fieldBeforeThink, frameRequest.myPlayerFrameRequest().field)) {
            LOG(INFO) << "FIELD INCONSISTENCY DETECTED: hand=" << hand_;
            VLOG(1) << '\n' << FieldPrettyPrinter::toStringFromMultipleFields(
                next1.fieldBeforeThink, frameRequest.myPlayerFrameRequest().kumipuyoSeq,
                frameRequest.myPlayerFrameRequest().field, frameRequest.myPlayerFrameRequest().kumipuyoSeq);

            next1.needsRethink = true;
        }

        // Rethink if necessary.
        if (next1.needsRethink || rethinkRequested_) {
            LOG(INFO) << "RETHINK";

            mergeField(&field_, frameRequest.myPlayerFrameRequest().field);
            const auto& kumipuyoSeq = frameRequest.myPlayerFrameRequest().kumipuyoSeq;

            KumipuyoSeq seq = rememberedSequence(hand_);
            CHECK_EQ(kumipuyoSeq.get(0), seq.get(0));
            CHECK_EQ(kumipuyoSeq.get(1), seq.get(1));

            next1.dropDecision = think(frameRequest.frameId, field_, seq, additionalThoughtInfo_, true);
            next1.kumipuyo = kumipuyoSeq.get(0);
            next1.ready = true;
            next1.needsRethink = false;
            rethinkRequested_ = false;
        }

        // Send
        connector_.send(FrameResponse(frameRequest.frameId, next1.dropDecision.decision(), next1.dropDecision.message()));

        // Move to next.
        if (next1.dropDecision.decision().isValid() && next1.kumipuyo.isValid()) {
            if (!field_.dropKumipuyo(next1.dropDecision.decision(), next1.kumipuyo)) {
                LOG(WARNING) << "failed to drop kumipuyo. Moving to impossible position?";
            }
            field_.simulate();
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
    hand_ = 0;
    rethinkRequested_ = false;
    field_.clear();
    seq_.clear();
    additionalThoughtInfo_ = AdditionalThoughtInfo();

    enemyHand_ = 0;
    enemyDecisionRequestFrameId_ = 0;
    enemyField_.clear();
    enemySeq_.clear();

    onGameWillBegin(frameRequest);
}

void AI::gameHasEnded(const FrameRequest& frameRequest)
{
    onGameHasEnded(frameRequest);
}

void AI::decisionRequested(const FrameRequest& frameRequest)
{
    // Don't check zenkeshi if hand_ is 0.
    if (hand_ != 0) {
        CoreField cf(frameRequest.myPlayerFrameRequest().field);
        if (cf.isZenkeshi())
            additionalThoughtInfo_.setHasZenkeshi(true);
    }

    ++hand_;
    onDecisionRequested(frameRequest);
}

void AI::grounded(const FrameRequest& frameRequest)
{
    CoreField field(frameRequest.myPlayerFrameRequest().field);
    field.forceDrop();
    RensaResult rensaResult = field.simulate();
    if (rensaResult.chains > 0) {
        additionalThoughtInfo_.setHasZenkeshi(false);
    }

    onGrounded(frameRequest);
}

void AI::next2Appeared(const FrameRequest& frameRequest)
{
    const KumipuyoSeq& kumipuyoSeq = frameRequest.myPlayerFrameRequest().kumipuyoSeq;

    for (int i = 0; i < 3; ++i) {
        if (hand_ + i < seq_.size()) {
            CHECK_EQ(seq_.get(hand_ + i), kumipuyoSeq.get(i))
                << hand_ << " " << i << " " << seq_.size()
                << seq_.toString() << " " << kumipuyoSeq.toString();
        } else {
            seq_.add(kumipuyoSeq.get(i));
        }
    }

    onNext2Appeared(frameRequest);
}

void AI::enemyDecisionRequested(const FrameRequest& frameRequest)
{
    if (enemyHand_ != 0) {
        CoreField cf(frameRequest.enemyPlayerFrameRequest().field);
        if (cf.isZenkeshi())
            additionalThoughtInfo_.setEnemyHasZenkeshi(true);
    }

    enemyHand_ += 1;
    enemyDecisionRequestFrameId_ = frameRequest.frameId;
    enemyField_ = CoreField(frameRequest.enemyPlayerFrameRequest().field);
    enemyField_.forceDrop();
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
        additionalThoughtInfo_.setOngoingRensa(rensaResult, frameRequest.frameId + rensaResult.frames);
        additionalThoughtInfo_.setEnemyHasZenkeshi(false);
    } else {
        additionalThoughtInfo_.unsetOngoingRensa();
    }

    onEnemyGrounded(frameRequest);
}

void AI::enemyNext2Appeared(const FrameRequest& frameRequest)
{
    const KumipuyoSeq& kumipuyoSeq = frameRequest.enemyPlayerFrameRequest().kumipuyoSeq;

    for (int i = 0; i < 3; ++i) {
        if (enemyHand_ + i < enemySeq_.size()) {
            CHECK_EQ(enemySeq_.get(enemyHand_ + i), kumipuyoSeq.get(i));
        } else {
            enemySeq_.add(kumipuyoSeq.get(i));
        }
    }

    gaze(enemyDecisionRequestFrameId_, enemyField_, rememberedSequence(enemyHand_));

    onEnemyNext2Appeared(frameRequest);
}

void AI::resetCurrentField(const CoreField& field)
{
    // TODO(mayah): Be more accurate.
    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        for (int y = 1; y <= CoreField::HEIGHT; ++y)
            field_.unsafeSet(x, y, field.color(x, y));

        for (int y = CoreField::HEIGHT + 1; y <= 14; ++y)
            field_.unsafeSet(x, y, EMPTY);

        field_.recalcHeightOn(x);

        // This might be useful for defensive players.
        if (behaviorDefensive_) {
            if (field_.color(x, CoreField::HEIGHT) != EMPTY) {
                while (field_.height(x) < 13)
                    field_.dropPuyoOn(x, OJAMA);
            }
        }
    }
}

// static
bool AI::isFieldInconsistent(const PlainField& f, const PlainField& provided)
{
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            if (f.get(x, y) != provided.get(x, y))
                return true;
        }
    }

    return false;
}

// static
void AI::mergeField(CoreField* f, const PlainField& provided)
{
    for (int x = 1; x <= 6; ++x) {
        bool restIsEmpty = false;
        int y;
        for (y = 1; y <= 12; ++y) {
            if (provided.get(x, y) == PuyoColor::EMPTY) {
                restIsEmpty = true;
                break;
            }
            f->unsafeSet(x, y, provided.get(x, y));
        }
        if (restIsEmpty) {
            for (; y <= 13; ++y) {
                DCHECK_EQ(PuyoColor::EMPTY, provided.get(x, y));
                f->unsafeSet(x, y, PuyoColor::EMPTY);
            }
        }
        f->recalcHeightOn(x);
    }
}

KumipuyoSeq AI::rememberedSequence(int indexFrom) const
{
    if (seq_.size() < enemySeq_.size())
        return enemySeq_.subsequence(indexFrom);
    else
        return seq_.subsequence(indexFrom);
}
