#ifndef DUEL_DUEL_SERVER_H_
#define DUEL_DUEL_SERVER_H_

#include <memory>
#include <string>
#include <vector>
#include <pthread.h>

#include "core/server/connector/game_log.h"

class ConnectorManager;
class Cui;
class GameState;
class GameStateObserver;
class UserInput;

class DuelServer {
public:
    // |dir| is the directory where the duel server program exsits.
    explicit DuelServer(const std::vector<std::string>& programNames);
    ~DuelServer();

    // Don't take ownership.
    void addObserver(GameStateObserver*);
    // TODO(mayah): Why public?
    void updateGameState(const GameState&);

    bool start();
    void stop();
    void join();

private:
    static void* runDuelLoopCallback(void*);
    void runDuelLoop();

    GameLog duel(ConnectorManager* manager, int* scores);

private:
    pthread_t th_;
    volatile bool shouldStop_;

    std::vector<std::string> programNames_;
    std::vector<GameStateObserver*> observers_;
    std::unique_ptr<UserInput> userInput_;
};

#endif
