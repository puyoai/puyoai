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
#include "duel/user_input.h"

#ifdef USE_SDL2
#include "gui/sdl_user_input.h"
#endif

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
    manager->Write(i, ss.str());
  }
}

static unique_ptr<UserInput> createUserInputIfNecessary()
{
#ifdef USE_SDL2
    if (FLAGS_use_gui)
        return unique_ptr<SDLUserInput>(new SDLUserInput);
#endif

    return unique_ptr<UserInput>(nullptr);
}

DuelServer::DuelServer(const vector<string>& programNames) :
    shouldStop_(false),
    programNames_(programNames),
    userInput_(createUserInputIfNecessary())
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
#if 0
    // Initialize Randomization.
    const char* seed_str = getenv("PUYO_SEED");
    unsigned int seed = seed_str ? atoi(seed_str) : time(NULL);
    srand(seed);
    std::cout << "seed=" << seed << std::endl;
    LOG(INFO) << "seed=" << seed;
#endif

    LOG(INFO) << "Starting duel server.";
    ConnectorManagerLinux manager(programNames_);

    int p1_win = 0;
    int p1_draw = 0;
    int p1_lose = 0;

#if 0
    bool log_result = false;
    string log_dir_str;
    const char* log_dir = getenv("PUYO_LOG_DIRECTORY");
    if (log_dir) {
        log_result = true;
        log_dir_str = string(log_dir);
        if (log_dir_str.substr(log_dir_str.size() - 1, 1) != "/") {
            log_dir_str += "/";
        }
    }
#endif

    int num_match = 0;
    while (!shouldStop_) {
        int scores[2];
        GameLog log = duel(&manager, scores);

#if 0
        if (log_result) {
            stringstream filename;
            filename << log_dir_str << num_match;
            ofstream ofs(filename.str().c_str(), ios::out);
            string output;
            log.SerializeToString(&output);
            ofs << "var data = " << output;
            ofs.close();
        }

        string filename;
        if (log_result) {
            stringstream ss;
            ss << log_dir_str << num_match;
            filename = ss.str();
        }
#endif

        string result = "";
        switch (log.result) {
        case P1_WIN:
            p1_win++;
            result = "P1_WIN";
            break;
        case P2_WIN:
            p1_lose++;
            result = "P2_WIN";
            break;
        case DRAW:
            p1_draw++;
            result = "DRAW";
            break;
        case P1_WIN_WITH_CONNECTION_ERROR:
            result = "P1_WIN_WITH_CONNECTION_ERROR";
            break;
        case P2_WIN_WITH_CONNECTION_ERROR:
            result = "P2_WIN_WITH_CONNECTION_ERROR";
            break;
        case PLAYING:
            LOG(FATAL) << "Game is still running?";
        }

#if 0
        map<string, string> results;
        results["result"] = result;
        results["logfile"] = filename;
        const char* user1 = getenv("PUYO_USER1");
        const char* user2 = getenv("PUYO_USER2");
        if (user1) results["user1"] = string(user1);
        if (user2) results["user2"] = string(user2);

        int is_first = true;
        cerr << "{";
        for (map<string, string>::iterator i = results.begin(); i != results.end(); i++) {
            if (!is_first) {
                cerr << ", ";
            }
            is_first = false;
            cerr << "\"" << i->first << "\": \"" << i->second << "\"";
        }
        cerr << "}" << endl;

        LOG(INFO) << result;
        cout << result << endl;
#endif

        cout << p1_win << " / " << p1_draw << " / " << p1_lose
             << " " << scores[0] << " vs " << scores[1] << endl;

#if 0
        seed = rand();
        srand(seed);
        LOG(INFO) << "seed=" << seed;
#endif

        if (log.result == P1_WIN_WITH_CONNECTION_ERROR ||
            log.result == P2_WIN_WITH_CONNECTION_ERROR ||
            p1_win == FLAGS_num_win || p1_lose == FLAGS_num_win ||
            p1_win + p1_draw + p1_lose == FLAGS_num_duel) {
            break;
        }
        num_match++;
    }
}

GameLog DuelServer::duel(ConnectorManager* manager, int* scores)
{
    for (auto observer : observers_)
        observer->newGameWillStart();

    LOG(INFO) << "Cpu managers started.";
    Game game(this, userInput_.get());
    LOG(INFO) << "Game started.";
    int current_id = 0;

    GameLog game_log;
    while (!shouldStop_) {
        // Timeout is 120s, and the game is 30fps.
        if (current_id >= FPS * 120) {
            game_log.result = GameResult::DRAW;
            return game_log;
        }
        // GO TO THE NEXT FRAME.
        current_id++;
        string player_info[2];
        game.GetFieldInfo(&player_info[0], &player_info[1]);
        SendInfo(manager, current_id, player_info);

        // CHECK IF THE GAME IS OVER.
        GameResult result = game.GetWinner(scores);
        if (result != PLAYING) {
            game_log.result = result;
            return game_log;
        }

        // READ INFO.
        // It takes up to 16ms to finish this section.
        vector<PlayerLog> all_data;
        if (!manager->GetActions(current_id, &all_data)) {
            if (manager->IsConnectorAlive(0)) {
                game_log.result = P1_WIN_WITH_CONNECTION_ERROR;
                game_log.error_log = manager->GetErrorLog();
                return game_log;
            } else {
                game_log.result = P2_WIN_WITH_CONNECTION_ERROR;
                game_log.error_log = manager->GetErrorLog();
                return game_log;
            }
        }

        // PLAY.
        game.Play(all_data, &game_log);
    }

    for (auto observer : observers_)
        observer->gameHasDone();

    return game_log;
}
