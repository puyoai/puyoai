#include "core/server/game_state_recorder.h"

#include <ctime>
#include <fstream>

#include <glog/logging.h>

#include "base/file.h"

using namespace std;

GameStateRecorder::GameStateRecorder(const string& dirPath) :
    recording_(false),
    dirPath_(dirPath)
{
}

void GameStateRecorder::newGameWillStart()
{
    time_t now;
    time(&now);

    struct tm ltm;
    localtime_r(&now, &ltm);

    char buf[1024];
    strftime(buf, 1024, "puyoai.gamestate.%Y%m%d-%H%M%S.json", &ltm);

    recording_ = true;
    filename_ = buf;

    LOG(INFO) << "will start game state logging to " << filename_;
}

void GameStateRecorder::onUpdate(const GameState& gameState)
{
    if (!recording_)
        return;

    gameStates_.push_back(gameState);
}

void GameStateRecorder::gameHasDone(GameResult)
{
    LOG(INFO) << "will emit game state to " << filename_;

    recording_ = false;
    vector<GameState> gs(std::move(gameStates_));
    gameStates_.clear();

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
