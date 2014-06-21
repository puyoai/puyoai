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

        Key key = field.GetKey(d);
        if (key != KEY_NONE) {
            *decision = d;
            return i;
        }
    }

    return -1;
}

static std::string FormatAckInfo(int ackFrameId, const vector<int>& nackFrameIds)
{
    stringstream nack;
    bool hasNack = false;
    for (size_t i = 0; i < nackFrameIds.size(); i++) {
        if (i != 0)
            nack << ",";
        nack << nackFrameIds[i];
        hasNack = true;
    }

    stringstream ret;
    if (ackFrameId > 0)
        ret << "ACK=" << ackFrameId << " ";
    if (hasNack)
        ret << "NACK=" << nack.str();
    return ret.str();
}

// TODO(mayah): This should not exist here.
// We must create FrameData, and pass it to connector.
void GetFieldInfo(const GameState& gameState, std::string* player1, std::string* player2)
{
    std::string f0 = gameState.field(0).GetFieldInfo();
    std::string f1 = gameState.field(1).GetFieldInfo();
    std::string y0 = gameState.field(0).GetYokokuInfo();
    std::string y1 = gameState.field(1).GetYokokuInfo();
    int state0 = gameState.field(0).userState().toDeprecatedState();
    int state1 = gameState.field(1).userState().toDeprecatedState();
    int score0 = gameState.field(0).score();
    int score1 = gameState.field(1).score();
    int ojama0 = gameState.field(0).ojama();
    int ojama1 = gameState.field(1).ojama();
    std::string ack0 = FormatAckInfo(gameState.ackFrameId(0), gameState.nackFrameIds(0));
    std::string ack1 = FormatAckInfo(gameState.ackFrameId(1), gameState.nackFrameIds(1));

    KumipuyoPos pos0 = gameState.field(0).kumipuyoPos();
    KumipuyoPos pos1 = gameState.field(1).kumipuyoPos();

    string win0, win1;
    GameResult result = gameState.gameResult();
    switch (result) {
    case GameResult::PLAYING:
        break;
    case GameResult::DRAW:
        win0 = "END=1 ";
        win1 = "END=-1 ";
        break;
    case GameResult::P1_WIN:
        win0 = "END=-1 ";
        win1 = "END=1 ";
        break;
    case GameResult::P2_WIN:
        win0 = win1 = "END=0 ";
        break;
    default:
        win0 = win1 = "END=0 ";
        break;
    }

    {
        std::stringstream ss;
        ss << "STATE=" << (state0 + (state1 << 1)) << " "
           << "YF=" << f0 << " "
           << "OF=" << f1 << " "
           << "YP=" << y0 << " "
           << "OP=" << y1 << " "
           << "YX=" << pos0.axisX() << " "
           << "YY=" << pos0.axisY() << " "
           << "YR=" << pos0.r << " "
           << "OX=" << pos1.axisX() << " "
           << "OY=" << pos1.axisY() << " "
           << "OR=" << pos1.r << " "
           << "YO=" << ojama0 << " "
           << "OO=" << ojama1 << " "
           << "YS=" << score0 << " "
           << "OS=" << score1 << " "
           << win0
           << ack0;
        *player1 = ss.str();
    }

    {
        std::stringstream ss;
        ss << "STATE=" << (state1 + (state0 << 1)) << " "
           << "YF=" << f1 << " "
           << "OF=" << f0 << " "
           << "YP=" << y1 << " "
           << "OP=" << y0 << " "
           << "YX=" << pos1.axisX() << " "
           << "YY=" << pos1.axisY() << " "
           << "YR=" << pos1.r << " "
           << "OX=" << pos0.axisX() << " "
           << "OY=" << pos0.axisY() << " "
           << "OR=" << pos0.r << " "
           << "YO=" << ojama1 << " "
           << "OO=" << ojama0 << " "
           << "YS=" << score1 << " "
           << "OS=" << score0 << " "
           << win1
           << ack1;
        *player2 = ss.str();
    }

    if (state0 & STATE_YOU_CAN_PLAY) {
        LOG(INFO) << "Player 0 can play.";
    }
    if (state1 & STATE_YOU_CAN_PLAY) {
        LOG(INFO) << "Player 1 can play.";
    }

    // TODO(koichi): Add ACK.
}

static void SendInfo(ConnectorManager* manager, int id, const GameState& gameState)
{
    string s[2];
    GetFieldInfo(gameState, &s[0], &s[1]);

    for (int i = 0; i < 2; i++) {
        stringstream ss;
        ss << "ID=" << id << " " << s[i];
        manager->connector(i)->write(ss.str());
    }
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

    int current_id = 0;
    GameResult gameResult = GameResult::GAME_HAS_STOPPED;
    while (!shouldStop_) {
        // Timeout is 120s, and the game is 30fps.
        if (current_id >= FPS * 120) {
            gameResult = GameResult::DRAW;
            break;
        }
        // GO TO THE NEXT FRAME.
        current_id++;
        //string player_info[2];
        //game.GetFieldInfo(gameState, &player_info[0], &player_info[1]);
        SendInfo(manager, current_id, gameState);

        // CHECK IF THE GAME IS OVER.
        GameResult result = gameState.gameResult();
        if (result != GameResult::PLAYING) {
            gameResult = result;
            break;
        }

        // READ INFO.
        // It takes up to 16ms to finish this section.
        vector<ConnectorFrameResponse> data[2];
        if (!manager->receive(current_id, data)) {
            if (manager->connector(0)->alive()) {
                gameResult = GameResult::P1_WIN_WITH_CONNECTION_ERROR;
                break;
            } else {
                gameResult = GameResult::P2_WIN_WITH_CONNECTION_ERROR;
                break;
            }
        }

        // PLAY.
        play(&gameState, data);
        for (GameStateObserver* observer : observers_)
            observer->onUpdate(gameState);
    }

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

        Key key = me->GetKey(gameState->decision(pi));
        if (accepted_index != -1 && data[pi][accepted_index].key != Key::KEY_NONE)
            key = data[pi][accepted_index].key;

        FrameContext context;
        me->PlayOneFrame(key, &context);
        context.apply(me, opponent);

        // Clear current key input if the move is done.
        if (me->userState().grounded)
            gameState->setDecision(pi, Decision::NoInputDecision());

        if (accepted_message != "")
            gameState->setMessage(pi, accepted_message);
    }
}
