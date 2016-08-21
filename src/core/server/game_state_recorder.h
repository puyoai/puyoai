#ifndef CORE_SERVER_GAME_STATE_RECORDER_H_
#define CORE_SERVER_GAME_STATE_RECORDER_H_

#include <string>
#include <vector>

#include "core/server/game_state.h"
#include "core/server/game_state_observer.h"

// GameStateRecorder records GameState, and emits as json for each game.
//
class GameStateRecorder : public GameStateObserver {
public:
    explicit GameStateRecorder(const std::string& dirPath,
                               bool record_only_p1_win = false);

    void newGameWillStart() override;
    void onUpdate(const GameState&) override;
    void gameHasDone(GameResult) override;

private:
    const bool record_only_p1_win_;
    bool recording_;
    std::string dirPath_;
    std::string filename_;
    std::vector<GameState> gameStates_;
};

#endif // CORE_SERVER_GAME_STATE_RECORDER_H_
