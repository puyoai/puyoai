#include "core/client/ai/ai.h"

#include "core/field/rensa_result.h"
#include "core/state.h"

using namespace std;

AI::AI(const string& name) :
    name_(name)
{
}

void AI::runLoop()
{
    bool needsSendDecision = false;
    bool needsSendDecisionNext = false;

    pair<DropDecision, Kumipuyo> decision;
    pair<DropDecision, Kumipuyo> decisionNext;

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

            const auto& kumipuyoSeq = frameData.myPlayerFrameData().kumipuyoSeq;
            // TODO(mayah): duel should have several frames before showing the first hand.
            // This workaround code should be removed after duel is changed.
            decision = make_pair(think(frameData.id, field_, KumipuyoSeq { kumipuyoSeq.get(1), kumipuyoSeq.get(2) }),
                                 kumipuyoSeq.get(1));
            needsSendDecision = true;

            if (decision.first.decision().isValid() && decision.second.isValid()) {
                field_.dropKumipuyo(decision.first.decision(), decision.second);
                field_.simulate();
            }

            decisionNext = make_pair(DropDecision(), Kumipuyo());
            needsSendDecisionNext = false;
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
            decision = make_pair(thinkFast(frameData.id, field_, KumipuyoSeq { kumipuyoSeq.get(0), kumipuyoSeq.get(1) }),
                                 kumipuyoSeq.get(0));
            if (decision.first.decision().isValid() && decision.second.isValid()) {
                field_.dropKumipuyo(decision.first.decision(), decision.second);
                field_.simulate();
            }
            needsSendDecision = true;
        }

        if (frameData.myPlayerFrameData().userState.wnextAppeared) {
            const auto& kumipuyoSeq = frameData.myPlayerFrameData().kumipuyoSeq;
            LOG(INFO) << "STATE_WNEXT_APPEARED";
            decisionNext = make_pair(think(frameData.id, field_, KumipuyoSeq { kumipuyoSeq.get(1), kumipuyoSeq.get(2) } ),
                                     kumipuyoSeq.get(1));
            needsSendDecisionNext = true;
        }
        if (frameData.myPlayerFrameData().userState.grounded) {
            LOG(INFO) << "STATE_YOU_GROUNDED";

            // TODO(mayah): When the game has started, STATE_YOU_GROUNDED comes several times.
            // This is a bug of Analyzer. After it is fixed, this if-condition should be removed.
            if (!needsSendDecision) {
                // TODO(mayah): If our AI is slow, this might cause some bug?
                needsSendDecision = needsSendDecisionNext;
                needsSendDecisionNext = false;

                decision = decisionNext;
                decisionNext = make_pair(DropDecision(), Kumipuyo());
                if (decision.first.decision().isValid() && decision.second.isValid()) {
                    field_.dropKumipuyo(decision.first.decision(), decision.second);
                    field_.simulate();
                }
            }
        }

        if (needsSendDecision && frameData.myPlayerFrameData().userState.playable) {
            needsSendDecision = false;
            connector_.send(frameData.id, decision.first);
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
