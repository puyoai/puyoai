#include "duel/replay_server.h"

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

#ifdef USE_SDL2
DECLARE_bool(use_gui);
#endif

struct ReplayServer::DuelState {
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

ReplayServer::ReplayServer(ConnectorManager* manager, const std::vector<GameState>& gameStates) :
    shouldStop_(false),
    manager_(manager),
    gameStates_(gameStates),
    pause_(false),
    thinking_(false)
{
}

ReplayServer::~ReplayServer()
{
}

void ReplayServer::addObserver(GameStateObserver* observer)
{
  DCHECK(observer);
  observers_.push_back(observer);
}

bool ReplayServer::start()
{
  th_ = thread([this](){
      this->runReplayLoop();
    });
  return true;
}

void ReplayServer::stop()
{
  shouldStop_ = true;
  if (th_.joinable())
    th_.join();
}

void ReplayServer::join()
{
  if (th_.joinable())
    th_.join();
}

void ReplayServer::runReplayLoop()
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

    num_match++;
  }

  if (callbackReplayServerWillExit_) {
    callbackReplayServerWillExit_();
  }
}

void ReplayServer::think(vector<FrameResponse> data[2]) {
  std::lock_guard<std::mutex> lock(mu_);
  pause_ = true;
  thinking_ = true;
  while (frameId_ >= 0) {
    if (gameStates_[frameId_].playerGameState(0).event.wnextAppeared) {
      break;
    }
    frameId_ -= 1;
  }
  
  if (!gameStates_[frameId_].playerGameState(0).event.wnextAppeared) {
    while (frameId_ < totalFrames()) {
      if (gameStates_[frameId_].playerGameState(0).event.wnextAppeared) {
        break;
      }
      frameId_ += 1;
    }
  }

  GameState gameState = gameStates_[frameId_];
  for (int pi = 0; pi < 2; ++pi) {
    manager_->connector(pi)->send(gameState.toFrameRequestFor(pi));
  }
  manager_->receive(gameState.frameId(), data);
  thinking_ = false;
}

GameResult ReplayServer::runGame(ConnectorManager* /*manager*/)
{
  for (auto observer : observers_)
    observer->newGameWillStart();

  GameResult gameResult = GameResult::GAME_HAS_STOPPED;
  frameId_ = 0;
  while (!shouldStop_) {
    GameState gameState = gameStates_[frameId_];
    for (GameStateObserver* observer : observers_)
      observer->onUpdate(gameState);

    gameResult = gameState.gameResult();

    usleep(16000);
    if (!pause_) {
      frameId_ += 1;
    }
    if (frameId_ >= int(gameStates_.size())) {
      frameId_ = 0;
    }
  }

  if (shouldStop_)
    gameResult = GameResult::GAME_HAS_STOPPED;

  for (auto observer : observers_)
    observer->gameHasDone(gameResult);

  return gameResult;
}
