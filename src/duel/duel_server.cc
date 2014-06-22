#include "duel/duel_server.h"

#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include <gflags/gflags.h>

#include "core/constant.h"
#include "core/decision.h"
#include "core/server/connector/connector.h"
#include "core/server/connector/connector_manager.h"
#include "core/sequence_generator.h"
#include "duel/frame_context.h"
#include "duel/game_state.h"
#include "duel/game_state_observer.h"

using namespace std;

DEFINE_int32(num_duel, -1, "After num_duel times of duel, the server will stop. negative is infinity.");
DEFINE_int32(num_win, -1, "After num_win times of 1p or 2p win, the server will stop. negative is infinity");
DEFINE_bool(use_even, true, "the match gets even after 2 minutes.");

#ifdef USE_SDL2
DECLARE_bool(use_gui);
#endif

/**
 * Updates decision when an applicable one is found.
 * Returns:
 *   if there is an accepted decision:
 *     its index in the given data array.
 *   else:
 *     -1
 */
static int updateDecision(const vector<ConnectorFrameResponse>& data, const FieldRealtime& field, Decision* decision)
{
    // Try all commands from the newest one.
    // If we find a command we can use, we'll ignore older ones.
    for (unsigned int i = data.size(); i > 0;) {
        i--;

        // When data contains key, it should be from HumanConnector.
        // In that case we accept it.
        if (data[i].key != Key::KEY_NONE)
            return i;

        Decision d = data[i].decision;

        // We don't send ACK/NACK for invalid decision.
        if (!d.isValid())
            continue;

        Key key = field.getKey(d);
        if (key != KEY_NONE) {
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
}

GameResult DuelServer::runGame(ConnectorManager* manager)
{
    for (auto observer : observers_)
        observer->newGameWillStart();

    KumipuyoSeq kumipuyoSeq = generateSequence();
    LOG(INFO) << "Puyo sequence=" << kumipuyoSeq.toString();

    GameState gameState(kumipuyoSeq);

    int frameId = 0;
    GameResult gameResult = GameResult::GAME_HAS_STOPPED;
    while (!shouldStop_) {
        frameId++;

        // --- Sends the current frame information.
        manager->send(gameState.toConnectorFrameRequest(frameId));

        // --- Reads the response of the current frame information.
        // It takes up to 16ms to finish this section.
        vector<ConnectorFrameResponse> data[2];
        if (!manager->receive(frameId, data)) {
            if (manager->connector(0)->alive()) {
                gameResult = GameResult::P1_WIN_WITH_CONNECTION_ERROR;
                break;
            } else {
                gameResult = GameResult::P2_WIN_WITH_CONNECTION_ERROR;
                break;
            }
        }

        // --- Play with input.
        play(&gameState, data);
        for (GameStateObserver* observer : observers_)
            observer->onUpdate(gameState);

        // --- Check the result
        // Timeout is 120s, and the game is 30fps.
        gameResult = gameState.gameResult();
        if (gameResult != GameResult::PLAYING)
            break;
        if (FLAGS_use_even && frameId >= FPS * 120) {
            gameResult = GameResult::DRAW;
            break;
        }
    }

    if (shouldStop_)
        gameResult = GameResult::GAME_HAS_STOPPED;

    for (auto observer : observers_)
        observer->gameHasDone();

    return gameResult;
}

void DuelServer::play(GameState* gameState, const vector<ConnectorFrameResponse> data[2])
{
    int ackFrameId[2];
    vector<int> nackFrameIds[2];

    for (int pi = 0; pi < 2; pi++) {
        FieldRealtime* me = gameState->mutableField(pi);
        FieldRealtime* opponent = gameState->mutableField(1 - pi);

        int accepted_index = updateDecision(data[pi], *me, gameState->mutableDecision(pi));

        // TODO(mayah): RecievedData from HumanConnector does not have any decision.
        // So, all data will be marked as NACK. Since the HumanConnector does not see ACK/NACK,
        // it's OK for now. However, this might cause future issues. Consider better way.

        // Take care of ack_info.
        for (size_t j = 0; j < data[pi].size(); j++) {
            const ConnectorFrameResponse& d = data[pi][j];

            // This case does not require ack.
            if (!d.isValid())
                continue;

            if (static_cast<int>(j) == accepted_index) {
                ackFrameId[pi] = d.frameId;
            } else {
                nackFrameIds[pi].push_back(d.frameId);
            }
        }

        string accepted_message;
        if (accepted_index != -1)
            accepted_message = data[pi][accepted_index].msg;

        Key key = me->getKey(gameState->decision(pi));
        if (accepted_index != -1 && data[pi][accepted_index].key != Key::KEY_NONE)
            key = data[pi][accepted_index].key;

        FrameContext context;
        me->playOneFrame(key, &context);
        context.apply(me, opponent);

        // Clear current key input if the move is done.
        if (me->userState().grounded)
            gameState->setDecision(pi, Decision::NoInputDecision());

        if (accepted_message != "")
            gameState->setMessage(pi, accepted_message);
    }
}
