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
    bool needsSend = false;
    bool hasGrounded = false;
};

AI::AI(int argc, char* argv[], const string& name) :
    name_(name)
{
    UNUSED_VARIABLE(argc);
    UNUSED_VARIABLE(argv);
}

void AI::runLoop()
{
    DecisionSending current;
    DecisionSending next;

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

            // Consider the current (EMPTY/EMPTY) puyo has grounded.
            current.clear();
            next.clear();
            current.hasGrounded = true;
        }

        // Update enemy info if necessary.
        if (frameData.enemyPlayerFrameData().userState.grounded)
            enemyGrounded(frameData);
        if (frameData.enemyPlayerFrameData().userState.wnextAppeared)
            enemyNext2Appeared(frameData);

        // Update my info if necessary.
        if (frameData.myPlayerFrameData().userState.ojamaDropped) {
            // We need to reconsider the current decision.
            LOG(INFO) << "STATE_OJAMA_DROPPED";
            resetCurrentField(frameData.myPlayerFrameData().field);
            const auto& kumipuyoSeq = frameData.myPlayerFrameData().kumipuyoSeq;

            current.dropDecision = thinkFast(frameData.id, field_, KumipuyoSeq { kumipuyoSeq.get(0), kumipuyoSeq.get(1) });
            current.kumipuyo = kumipuyoSeq.get(0);
            current.needsSend = true;
            current.hasGrounded = false;

            if (current.dropDecision.decision().isValid() && current.kumipuyo.isValid()) {
                field_.dropKumipuyo(current.dropDecision.decision(), current.kumipuyo);
                field_.simulate();
            }
        }

        // STATE_YOU_GROUNDED and STATE_WNEXT_APPEARED might come out-of-order.

        if (frameData.myPlayerFrameData().userState.wnextAppeared) {
            const auto& kumipuyoSeq = frameData.myPlayerFrameData().kumipuyoSeq;
            LOG(INFO) << "STATE_WNEXT_APPEARED";

            next.dropDecision = think(frameData.id, field_, KumipuyoSeq { kumipuyoSeq.get(1), kumipuyoSeq.get(2) } );
            next.kumipuyo = kumipuyoSeq.get(1);
            next.needsSend = true;
            next.hasGrounded = false;
        }

        if (frameData.myPlayerFrameData().userState.grounded) {
            LOG(INFO) << "STATE_YOU_GROUNDED";
            current.hasGrounded = true;
        }

        // When the current puyo is grounded and the next hand is considered, we will move to the next hand.
        if (current.hasGrounded && next.needsSend) {
            CHECK(!current.needsSend);
            current = next;
            next.clear();

            if (current.dropDecision.decision().isValid() && current.kumipuyo.isValid()) {
                field_.dropKumipuyo(current.dropDecision.decision(), current.kumipuyo);
                field_.simulate();
            }
        }

        if (current.needsSend && frameData.myPlayerFrameData().userState.playable) {
            connector_.send(frameData.id, current.dropDecision);
            current.needsSend = false;
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
