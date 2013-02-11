#ifndef GAME_H_
#define GAME_H_

#include <string>
#include <vector>
#include "field.h"
#include "plan.h"

struct PlayerState {
    Field field;
    std::vector<KumiPuyo> kumiPuyos;
    int score;
    int ojama;
};

class Game {
public:
    Game();
    ~Game();

    bool shouldInitialize() const;

    bool shouldThink() const;
    bool canPlay() const;

    bool enemyHasPutPuyo() const;

    const PlayerState& myPlayerState() const { return playerStates[0]; }
    const PlayerState& enemyPlayerState() const { return playerStates[1]; }

public:
    int id;
    unsigned long long state;
    PlayerState playerStates[2];
};

#endif
