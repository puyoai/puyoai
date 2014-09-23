#include "core/client/ai/ai.h"

#include "core/client/connector/client_frame_response.h"
#include "core/field/rensa_result.h"
#include "core/field_pretty_printer.h"
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
    // True when we need to reconsider this hand. This happens when we detected ojama etc.
    bool needsReconsider = false;
};

AI::AI(int argc, char* argv[], const string& name) :
    name_(name),
    hand_(0),
    reconsiderRequested_(false),
    difensive_(false)
{
    UNUSED_VARIABLE(argc);
    UNUSED_VARIABLE(argv);
}

// TODO(mayah): Consider to introduce state. It's hard to maintain flags.
void AI::runLoop()
{
    DecisionSending next1;

    while (true) {
        google::FlushLogFiles(google::INFO);

        FrameData frameData = connector_.receive();
        if (frameData.connectionLost) {
            LOG(INFO) << "connection lost";
            break;
        }

        if (!frameData.valid) {
            connector_.send(ClientFrameResponse(frameData.id));
            continue;
        }

        if (frameData.hasGameEnd()) {
            gameHasEnded(frameData);
        }
        // Before starting a new game, we need to consider the first hand.
        // TODO(mayah): Maybe game server should send some information that we should initialize.
        if (frameData.id == 1) {
            hand_ = 0;
            reconsiderRequested_ = false;
            field_.clear();
            gameWillBegin(frameData);
            next1.clear();
        }
        // Update enemy info if necessary.
        if (frameData.enemyPlayerFrameData().userState.grounded)
            enemyGrounded(frameData);
        if (frameData.enemyPlayerFrameData().userState.wnextAppeared)
            enemyNext2Appeared(frameData);

        // STATE_YOU_GROUNDED and STATE_WNEXT_APPEARED might come out-of-order.
        if (frameData.myPlayerFrameData().userState.wnextAppeared) {
            const auto& kumipuyoSeq = frameData.myPlayerFrameData().kumipuyoSeq;
            LOG(INFO) << "STATE_WNEXT_APPEARED";
            VLOG(1) << '\n' << field_.toDebugString();

            next1.fieldBeforeThink = field_;
            next1.dropDecision = think(frameData.id, field_, KumipuyoSeq { kumipuyoSeq.get(1), kumipuyoSeq.get(2) }, AdditionalThoughtInfo());
            next1.kumipuyo = kumipuyoSeq.get(1);
            next1.ready = true;
        }

        // Update my info if necessary.
        if (frameData.myPlayerFrameData().userState.ojamaDropped) {
            // We need to reconsider the next1 decision.
            next1.needsReconsider = true;
        }

        if (frameData.myPlayerFrameData().userState.decisionRequest) {
            VLOG(1) << "REQUESTED";
            next1.requested = true;
        }

        if (!next1.requested || !next1.ready) {
            connector_.send(ClientFrameResponse(frameData.id));
            continue;
        }

        // Check field inconsistency. We only check when hand_ >= 3, since we cannot trust the field in 3 hands.
        if (hand_ >= 3) {
            if (!next1.fieldBeforeThink.isVisibllySame(frameData.myPlayerFrameData().field)) {
                LOG(INFO) << "FIELD INCONSISTENCY DETECTED: hand=" << hand_;
                VLOG(1) << '\n' << FieldPrettyPrinter::toStringFromMultipleFields(
                    next1.fieldBeforeThink, frameData.myPlayerFrameData().kumipuyoSeq,
                    frameData.myPlayerFrameData().field, frameData.myPlayerFrameData().kumipuyoSeq);

                next1.needsReconsider = true;
            }
        }

        // Reconsider if necessary.
        if (next1.needsReconsider || reconsiderRequested_) {
            LOG(INFO) << "RECONSIDER";

            resetCurrentField(frameData.myPlayerFrameData().field);
            const auto& kumipuyoSeq = frameData.myPlayerFrameData().kumipuyoSeq;
            next1.dropDecision = thinkFast(frameData.id, field_, KumipuyoSeq { kumipuyoSeq.get(0), kumipuyoSeq.get(1) }, AdditionalThoughtInfo());
            next1.kumipuyo = kumipuyoSeq.get(0);
            next1.ready = true;
            next1.needsReconsider = false;
            reconsiderRequested_ = false;
        }

        // Send
        ++hand_;
        connector_.send(ClientFrameResponse(frameData.id, next1.dropDecision.decision(), next1.dropDecision.message()));

        // Move to next.
        if (next1.dropDecision.decision().isValid() && next1.kumipuyo.isValid()) {
            field_.dropKumipuyo(next1.dropDecision.decision(), next1.kumipuyo);
            field_.simulate();
        }
        next1.clear();
    }

    LOG(INFO) << "will exit run loop";
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
        if (difensive_) {
            if (field_.color(x, CoreField::HEIGHT) != EMPTY) {
                while (field_.height(x) < 13)
                    field_.dropPuyoOn(x, OJAMA);
            }
        }
    }
}

