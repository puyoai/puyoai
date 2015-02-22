#ifndef DUEL_DUEL_SERVER_H_
#define DUEL_DUEL_SERVER_H_

#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "core/game_result.h"

class ConnectorManager;
class GameStateObserver;
struct FrameResponse;

class DuelServer {
public:
    explicit DuelServer(ConnectorManager*);
    ~DuelServer();

    // Doesn't take ownership.
    void addObserver(GameStateObserver*);

    bool start();
    void stop();
    void join();

    // This callback should be alive during duel server is alive.
    void setCallbackDuelServerWillExit(std::function<void ()> callback)
    {
        callbackDuelServerWillExit_ = callback;
    }

private:
    struct DuelState;

    void runDuelLoop();
    void play(DuelState*, const std::vector<FrameResponse> data[2]);

    GameResult runGame(ConnectorManager* manager);

private:
    std::thread th_;
    volatile bool shouldStop_;

    ConnectorManager* manager_;
    std::vector<GameStateObserver*> observers_;
    std::function<void ()> callbackDuelServerWillExit_;
};

#endif
