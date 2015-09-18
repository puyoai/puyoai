#include "duel/duel_server.h"

#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include <gflags/gflags.h>

#include "core/decision.h"
#include "core/frame_response.h"
#include "core/kumipuyo_seq_generator.h"
#include "core/puyo_controller.h"
#include "core/server/connector/connector.h"
#include "core/server/connector/connector_manager.h"
#include "core/server/game_state.h"
#include "core/server/game_state_observer.h"
#include "duel/field_realtime.h"
#include "duel/frame_context.h"

using namespace std;

DEFINE_int32(num_duel, -1, "After num_duel times of duel, the server will stop. negative is infinity.");
DEFINE_int32(num_win, -1, "After num_win times of 1p or 2p win, the server will stop. negative is infinity");
DEFINE_bool(use_even, false, "the match gets even after 2 minutes.");

#ifdef USE_SDL2
DECLARE_bool(use_gui);
#endif

struct DuelServer::DuelState {
    explicit DuelState(const KumipuyoSeq& seq) : field { FieldRealtime(0, seq), FieldRealtime(1, seq) } {}

    GameState toGameState() const
    {
        GameState gs(frameId);
        for (int pi = 0; pi < 2; ++pi) {
            PlayerGameState* pgs = gs.mutablePlayerGameState(pi);
            const FieldRealtime& fr = field[pi];
            pgs->field = fr.field();
            pgs->kumipuyoSeq = fr.visibleKumipuyoSeq();
            pgs->kumipuyoPos = fr.kumipuyoPos();
            pgs->event = fr.userEvent();
            pgs->dead = fr.isDead();
            pgs->playable = fr.playable();
            pgs->score = fr.score();
            pgs->pendingOjama = fr.numPendingOjama();
            pgs->fixedOjama = fr.numFixedOjama();
            pgs->decision = decision[pi];
            pgs->message = message[pi];
        }

        return gs;
    }

    int frameId = 0;
    FieldRealtime field[2];
    Decision decision[2];
    string message[2];
};

/**
 * Updates decision when an applicable one is found.
 * Returns:
 *   if there is an accepted decision:
 *     its index in the given data array.
 *   else:
 *     -1
 */
static int updateDecision(int frameId, const vector<FrameResponse>& data, const FieldRealtime& field, Decision* decision)
{
    // updateDecision is called when grounded. Chigiri-puyo might be in the air.
    CoreField cf(CoreField::fromPlainFieldWithDrop(field.field()));

    // Try all commands from the newest one.
    // If we find a command we can use, we'll ignore older ones.
    for (unsigned int i = data.size(); i > 0;) {
        i--;

        // Probably we got the previous game's response. We should ignore it.
        if (data[i].frameId > frameId) {
            LOG(WARNING) << "Get previous game response? frameId=" << frameId << " response=" << data[i].toString();
            continue;
        }

        // When data contains key, it should be from HumanConnector.
        // In that case we accept it.
        if (data[i].keySet.hasSomeKey())
            return i;

        Decision d = data[i].decision;

        // We don't send ACK/NACK for invalid decision.
        if (!d.isValid())
            continue;

        if (PuyoController::isReachableFrom(cf, field.kumipuyoMovingState(), d)) {
            *decision = d;
            return i;
        }
    }

    return -1;
}

DuelServer::DuelServer(ConnectorManager* manager) :
    shouldStop_(false),
    manager_(manager)
{
}

DuelServer::~DuelServer()
{
}

void DuelServer::addObserver(GameStateObserver* observer)
{
    DCHECK(observer);
    observers_.push_back(observer);
}

bool DuelServer::start()
{
    th_ = thread([this](){
        this->runDuelLoop();
    });
    return true;
}

void DuelServer::stop()
{
    shouldStop_ = true;
    if (th_.joinable())
        th_.join();
}

void DuelServer::join()
{
    if (th_.joinable())
        th_.join();
}

void DuelServer::runDuelLoop()
{
    int p1_win = 0;
    int p1_draw = 0;
    int p1_lose = 0;
    int num_match = 0;

    while (!shouldStop_) {
        GameResult gameResult = runGame(manager_);

        string result = "";
        switch (gameResult) {
        case GameResult::P1_WIN:
            p1_win++;
            result = "P1_WIN";
            break;
        case GameResult::P2_WIN:
            p1_lose++;
            result = "P2_WIN";
            break;
        case GameResult::DRAW:
            p1_draw++;
            result = "DRAW";
            break;
        case GameResult::P1_WIN_WITH_CONNECTION_ERROR:
            result = "P1_WIN_WITH_CONNECTION_ERROR";
            break;
        case GameResult::P2_WIN_WITH_CONNECTION_ERROR:
            result = "P2_WIN_WITH_CONNECTION_ERROR";
            break;
        case GameResult::PLAYING:
            LOG(FATAL) << "Game is still running?";
            break;
        case GameResult::GAME_HAS_STOPPED:
            // Game has stopped.
            return;
        }

        cout << p1_win << " / " << p1_draw << " / " << p1_lose << endl;

        if (gameResult == GameResult::P1_WIN_WITH_CONNECTION_ERROR ||
            gameResult == GameResult::P2_WIN_WITH_CONNECTION_ERROR ||
            p1_win == FLAGS_num_win || p1_lose == FLAGS_num_win ||
            p1_win + p1_draw + p1_lose == FLAGS_num_duel) {
            break;
        }

        num_match++;
    }

    if (callbackDuelServerWillExit_) {
        callbackDuelServerWillExit_();
    }
}

GameResult DuelServer::runGame(ConnectorManager* manager)
{
    for (auto observer : observers_)
        observer->newGameWillStart();

    KumipuyoSeq kumipuyoSeq = KumipuyoSeqGenerator::generateACPuyo2Sequence();

    LOG(INFO) << "Puyo sequence=" << kumipuyoSeq.toString();

    DuelState duelState(kumipuyoSeq);

    GameResult gameResult = GameResult::GAME_HAS_STOPPED;
    while (!shouldStop_) {
        duelState.frameId += 1;
        int frameId = duelState.frameId;

        GameState gameState = duelState.toGameState();

        // --- Sends the current frame information.
        for (int pi = 0; pi < 2; ++pi) {
            manager->connector(pi)->send(gameState.toFrameRequestFor(pi));
        }

        // --- Reads the response of the current frame information.
        // It takes up to 1/FPS [s] to finish this section.
        vector<FrameResponse> data[2];
        if (!manager->receive(frameId, data)) {
            if (manager->connector(0)->isClosed()) {
                gameResult = GameResult::P2_WIN_WITH_CONNECTION_ERROR;
                break;
            } else {
                gameResult = GameResult::P1_WIN_WITH_CONNECTION_ERROR;
                break;
            }
        }

        // --- Play with input.
        play(&duelState, data);
        gameState = duelState.toGameState();
        for (GameStateObserver* observer : observers_)
            observer->onUpdate(gameState);

        // --- Check the result
        gameResult = gameState.gameResult();
        if (gameResult != GameResult::PLAYING) {
            break;
        }

        // Timeout.
        if (FLAGS_use_even && frameId >= FPS * 120) {
            gameResult = GameResult::DRAW;
            break;
        }
    }

    if (shouldStop_)
        gameResult = GameResult::GAME_HAS_STOPPED;

    // Send Request for GameResult.
    {
        ++duelState.frameId;
        GameState gameState = duelState.toGameState();
        for (int pi = 0; pi < 2; ++pi) {
            manager->connector(pi)->send(gameState.toFrameRequestFor(pi));
        }
    }

    for (auto observer : observers_)
        observer->gameHasDone(gameResult);

    return gameResult;
}

void DuelServer::play(DuelState* duelState, const vector<FrameResponse> data[2])
{
    for (int pi = 0; pi < 2; pi++) {
        FieldRealtime* me = &duelState->field[pi];
        FieldRealtime* opponent = &duelState->field[1 - pi];

        int accepted_index = updateDecision(duelState->frameId, data[pi], *me, &duelState->decision[pi]);

        // TODO(mayah): ReceivedData from HumanConnector does not have any decision.
        // So, all data will be marked as NACK. Since the HumanConnector does not see ACK/NACK,
        // it's OK for now. However, this might cause future issues. Consider better way.

        if (accepted_index != -1) {
            KeySetSeq kss = PuyoController::findKeyStrokeFrom(CoreField(me->field()), me->kumipuyoMovingState(), duelState->decision[pi]);
            me->setKeySetSeq(kss);
        }

        string acceptedMessage;
        if (accepted_index != -1)
            acceptedMessage = data[pi][accepted_index].message;

        LOG(INFO) << "Current KeySetSeq: " << pi << " " << me->keySetSeq().toString();
        KeySet keySet = me->frontKeySet();
        me->dropFrontKeySet();
        // For human connector. The received data from HumanConnector might have some key.
        if (accepted_index != -1 && data[pi][accepted_index].keySet.hasSomeKey()) {
            keySet = data[pi][accepted_index].keySet;
        }

        FrameContext context;
        me->playOneFrame(keySet, &context);
        context.apply(me, opponent);

        // Clear current key input if the move is done.
        if (me->userEvent().grounded) {
            duelState->decision[pi] = Decision();
            me->setKeySetSeq(KeySetSeq());
        }

        if (!acceptedMessage.empty()) {
            duelState->message[pi] = acceptedMessage;
        }
    }
}
