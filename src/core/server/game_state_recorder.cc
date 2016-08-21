#include "core/server/game_state_recorder.h"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>

#include <glog/logging.h>

#include "base/file/path.h"

using namespace std;

GameStateRecorder::GameStateRecorder(const string& dirPath,
                                     bool record_only_p1_win) :
    record_only_p1_win_(record_only_p1_win),
    recording_(false),
    dirPath_(dirPath)
{
}

void GameStateRecorder::newGameWillStart()
{
    time_t now;
    time(&now);

#if defined(_MSC_VER)
    ostringstream oss;
    oss << std::put_time(std::localtime(&now), "puyoai.gamestate.%Y%m%d-%H%M%S.json");

    filename_ = oss.str();
#else
    // Clang currently does not support std::put_time.
    // TODO: Remove this #else when clang supports it.
    struct tm ltm;
    localtime_r(&now, &ltm);

    char buf[1024];
    strftime(buf, 1024, "puyoai.gamestate.%Y%m%d-%H%M%S.json", &ltm);

    filename_ = buf;
#endif
    recording_ = true;

    LOG(INFO) << "will start game state logging to " << filename_;
}

void GameStateRecorder::onUpdate(const GameState& gameState)
{
    if (!recording_)
        return;

    gameStates_.push_back(gameState);
}

void GameStateRecorder::gameHasDone(GameResult gameResult)
{
    recording_ = false;
    vector<GameState> gs(std::move(gameStates_));
    gameStates_.clear();

    if (record_only_p1_win_) {
        if (gameResult != GameResult::P1_WIN) {
            LOG(INFO) << "game state won't be emitted since P1 didn't win";
            return;
        }
    }

    LOG(INFO) << "will emit game state to " << filename_;

    // emits json here.
    const string path = file::joinPath(dirPath_, filename_);
    ofstream fs(path);
    if (!fs) {
        PLOG(ERROR) << "couldn't open game state record path: " << path;
        return;
    }

    fs << "[";
    for (size_t i = 0; i < gs.size(); ++i) {
        if (i > 0) {
            fs << "," << endl;
        }
        fs << gs[i].toJson();
    }
    fs << "]";
}
