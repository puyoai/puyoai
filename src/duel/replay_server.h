#ifndef DUEL_REPLAY_SERVER_H_
#define DUEL_REPLAY_SERVER_H_

#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "core/frame_response.h"
#include "core/game_result.h"
#include "core/server/game_state.h"

class ConnectorManager;
class GameStateObserver;

class ReplayServer {
public:
  ReplayServer(ConnectorManager*, const std::vector<GameState>& gameStates);
  ~ReplayServer();

  // Doesn't take ownership.
  void addObserver(GameStateObserver*);

  bool start();
  void stop();
  void join();

  // This callback should be alive during duel server is alive.
  void setCallbackDuelServerWillExit(std::function<void ()> callback)
  {
    callbackReplayServerWillExit_ = callback;
  }

  int frameId() const {
    return frameId_;
  }

  void setFrameId(int frameId) {
    std::lock_guard<std::mutex> lock(mu_);
    frameId_ = frameId;
  }

  int totalFrames() const {
    return gameStates_.back().frameId();
  }

  void togglePlayState() {
    std::lock_guard<std::mutex> lock(mu_);
    pause_ = !pause_;
  }

  bool pause() const {
    return pause_;
  }

  bool thinking() const {
    return thinking_;
  }

  GameState nowGameState() const {
    return gameStates_[frameId()];
  }

  void think(std::vector<FrameResponse> data[2]);

private:
  struct DuelState;

  void runReplayLoop();

  GameResult runGame(ConnectorManager* manager);

private:
  std::mutex mu_;
  std::thread th_;
  volatile bool shouldStop_;

  ConnectorManager* manager_;
  std::vector<GameStateObserver*> observers_;
  std::vector<GameState> gameStates_;
  std::function<void ()> callbackReplayServerWillExit_;

  int frameId_;
  bool pause_;
  bool thinking_;
};

#endif
