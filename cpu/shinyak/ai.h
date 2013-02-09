#ifndef __AI_H_
#define __AI_H_

#include <fstream>
#include <string>
#include <vector>

#include "player_info.h"

class Game;
class Decision;
class Field;
class KumiPuyo;
class Plan;

class AI {
public:
    AI(const std::string& name);

    std::string getName() const;

    void initialize(const Game&);

    void think(Decision& result, const Game&);
    void myRensaFinished(const Game&);
    void myOjamaDropped(const Game&);

    void enemyWNextAppeared(const Game&);
    void enemyGrounded(const Game&);

private:
    void decide(const Game&, Decision*);
    double eval(int currentFrameId, const Plan&) const;

private:
    std::string m_name;
    mutable std::ofstream log;

    MyPlayerInfo m_myPlayerInfo;
    EnemyInfo m_enemyInfo;
};

#endif
