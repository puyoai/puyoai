#ifndef DUEL_DUEL_SERVER_H_
#define DUEL_DUEL_SERVER_H_

#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "core/server/connector/connector_frame_response.h"
#include "duel/game_result.h"

class ConnectorManager;
class GameState;
class GameStateObserver;

class DuelServer {
public:
    explicit DuelServer(ConnectorManager*);
    ~DuelServer();

    // Don't take ownership.
    void addObserver(GameStateObserver*);

    bool start();
    void stop();
    void join();

private:
    void runDuelLoop();
    void play(GameState*, const std::vector<ConnectorFrameResponse> data[2]);

    GameResult runGame(ConnectorManager* manager);

private:
    std::thread th_;
    volatile bool shouldStop_;

    ConnectorManager* manager_;
    std::vector<GameStateObserver*> observers_;
};

#endif
