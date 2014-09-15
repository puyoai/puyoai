#include "core/client/ai/ai.h"

#include "core/field/rensa_result.h"
#include "core/state.h"

using namespace std;

struct DecisionSending {
    void clear()
    {
        *this = DecisionSending();
    }

    DropDecision dropDecision = DropDecision();
    Kumipuyo kumipuyo = Kumipuyo();

    // After we've decided the decision, needsSend will be true.
    bool needsSend = false;
    // True when we need to reconsider this hand. This happens when we detected ojama etc.
    bool needsReconsider = false;
    bool prevHasGrounded = false;
};

AI::AI(int argc, char* argv[], const string& name) :
    name_(name)
{
    UNUSED_VARIABLE(argc);
    UNUSED_VARIABLE(argv);
}

void AI::runLoop()
{
    DecisionSending next1;
    DecisionSending next2;

    while (true) {
        FrameData frameData = connector_.receive();
        if (frameData.connectionLost) {
            LOG(INFO) << "connection lost";
            break;
        }

        if (!frameData.valid) {
            connector_.sendWithoutDecision(frameData.id);
            continue;
        }

        if (frameData.hasGameEnd()) {
            gameHasEnded(frameData);
        }

        // Before starting a new game, we need to consider the first hand.
        // TODO(mayah): Maybe game server should send some information that we should initialize.
        if (frameData.id == 1) {
            gameWillBegin(frameData);
            field_.clear();

            next1.clear();
            next2.clear();
        }

        // Update enemy info if necessary.
        if (frameData.enemyPlayerFrameData().userState.grounded)
            enemyGrounded(frameData);
        if (frameData.enemyPlayerFrameData().userState.wnextAppeared)
            enemyNext2Appeared(frameData);

        // Update my info if necessary.
        if (frameData.myPlayerFrameData().userState.ojamaDropped) {
            // We need to reconsider the next1 decision.
            next1.needsReconsider = true;
        }

        // STATE_YOU_GROUNDED and STATE_WNEXT_APPEARED might come out-of-order.
        if (frameData.myPlayerFrameData().userState.wnextAppeared) {
            const auto& kumipuyoSeq = frameData.myPlayerFrameData().kumipuyoSeq;
            LOG(INFO) << "STATE_WNEXT_APPEARED";

            next2.dropDecision = think(frameData.id, field_, KumipuyoSeq { kumipuyoSeq.get(1), kumipuyoSeq.get(2) } );
            next2.kumipuyo = kumipuyoSeq.get(1);
            next2.needsSend = true;
            next2.prevHasGrounded = false;
        }

        if (frameData.myPlayerFrameData().userState.grounded) {
            LOG(INFO) << "STATE_YOU_GROUNDED";
            next1.prevHasGrounded = true;
        }

        LOG(INFO) << "FRAME: " << frameData.id
                  << " NEXT1: prev_grounded=" << next1.prevHasGrounded
                  << " needsSend=" << next1.needsSend
                  << " NEXT2: prev_grounded=" << next2.prevHasGrounded
                  << " needsSend=" << next2.needsSend;

        // When the current puyo is grounded and the next hand is considered, we will move to the next hand.
        if (!next1.needsReconsider && next1.prevHasGrounded && next2.needsSend) {
            CHECK(!next1.needsSend) << "client/ai bug or your AI is too slow!";
            next1 = next2;
            next2.clear();

            if (next1.dropDecision.decision().isValid() && next1.kumipuyo.isValid()) {
                field_.dropKumipuyo(next1.dropDecision.decision(), next1.kumipuyo);
                field_.simulate();
            }
        }

        if (frameData.myPlayerFrameData().userState.playable && next1.needsReconsider) {
            LOG(INFO) << "RECONSIDER";

            resetCurrentField(frameData.myPlayerFrameData().field);
            const auto& kumipuyoSeq = frameData.myPlayerFrameData().kumipuyoSeq;
            next1.dropDecision = thinkFast(frameData.id, field_, KumipuyoSeq { kumipuyoSeq.get(0), kumipuyoSeq.get(1) });
            next1.kumipuyo = kumipuyoSeq.get(0);
            next1.needsSend = true;

            if (next1.dropDecision.decision().isValid() && next1.kumipuyo.isValid()) {
                field_.dropKumipuyo(next1.dropDecision.decision(), next1.kumipuyo);
                field_.simulate();
            }
        }

        if (next1.needsSend && frameData.myPlayerFrameData().userState.playable) {
            connector_.send(frameData.id, next1.dropDecision);
            next1.needsSend = false;
            continue;
        }

        connector_.sendWithoutDecision(frameData.id);
        google::FlushLogFiles(google::INFO);
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
