#include "duel/duel_server.h"

#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include <gflags/gflags.h>

#include "core/constant.h"
#include "core/decision.h"
#include "core/server/connector/connector_manager.h"
#include "core/server/connector/connector_manager_linux.h"
#include "duel/game.h"
#include "duel/game_state.h"
#include "duel/game_state_observer.h"

using namespace std;

DEFINE_int32(num_duel, -1, "After num_duel times of duel, the server will stop. negative is infinity.");
DEFINE_int32(num_win, -1, "After num_win times of 1p or 2p win, the server will stop. negative is infinity");

#ifdef USE_SDL2
DECLARE_bool(use_gui);
#endif

static void SendInfo(ConnectorManager* manager, int id, string status[2])
{
    for (int i = 0; i < 2; i++) {
        stringstream ss;
        ss << "ID=" << id << " " << status[i].c_str();
        manager->connector(i)->write(ss.str());
    }
}

DuelServer::DuelServer(const vector<string>& programNames) :
    shouldStop_(false),
    programNames_(programNames)
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

void DuelServer::updateGameState(const GameState& state)
{
    for (GameStateObserver* observer : observers_)
        observer->onUpdate(state);
}

bool DuelServer::start()
{
    CHECK(pthread_create(&th_, nullptr, runDuelLoopCallback, this) == 0);
    return true;
}

void DuelServer::stop()
{
    shouldStop_ = true;
}
void DuelServer::join()
{
    CHECK(pthread_join(th_, nullptr) == 0);
}

// static
void* DuelServer::runDuelLoopCallback(void* p)
{
    DuelServer* server = reinterpret_cast<DuelServer*>(p);
    server->runDuelLoop();

    return nullptr;
}

void DuelServer::runDuelLoop()
{
    ConnectorManagerLinux manager {
        Connector::create(0, programNames_[0]),
        Connector::create(1, programNames_[1])
    };

    int p1_win = 0;
    int p1_draw = 0;
    int p1_lose = 0;
    int num_match = 0;

    while (!shouldStop_) {
        GameResult gameResult = duel(&manager);

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

GameResult DuelServer::duel(ConnectorManager* manager)
{
    for (auto observer : observers_)
        observer->newGameWillStart();

    Game game(this);

    LOG(INFO) << "Game has started.";

    int current_id = 0;
    GameResult gameResult;
    while (!shouldStop_) {
        // Timeout is 120s, and the game is 30fps.
        if (current_id >= FPS * 120) {
            gameResult = GameResult::DRAW;
            break;
        }
        // GO TO THE NEXT FRAME.
        current_id++;
        string player_info[2];
        game.GetFieldInfo(&player_info[0], &player_info[1]);
        SendInfo(manager, current_id, player_info);

        // CHECK IF THE GAME IS OVER.
        GameResult result = game.GetWinner();
        if (result != GameResult::PLAYING) {
            gameResult = result;
            break;
        }

        // READ INFO.
        // It takes up to 16ms to finish this section.
        vector<ReceivedData> data[2];
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
        game.Play(data);
    }

    for (auto observer : observers_)
        observer->gameHasDone();

    return gameResult;
}
