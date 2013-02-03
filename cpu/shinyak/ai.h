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
    std::string getName() const;

    void initialize(const Game&);

    void think(Decision& result, const Game&, std::ofstream& log);
    void myRensaFinished(const Game&);
    void myOjamaDropped(const Game&);

    void enemyWNextAppeared(const Game&);
    void enemyGrounded(const Game&);

private:
    void decide(const Game&, Decision*, std::ofstream& log);
    double eval(int currentFrameId, const Plan&, std::ofstream& log) const;

private:
    MyPlayerInfo m_myPlayerInfo;
    EnemyInfo m_enemyInfo;
};

#endif
