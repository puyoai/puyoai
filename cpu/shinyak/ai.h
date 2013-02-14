#ifndef __AI_H_
#define __AI_H_

#include <fstream>
#include <string>
#include <vector>

#include "enemy_info.h"
#include "player_info.h"

class Game;
class Decision;
class DropDecision;
class Field;
class KumiPuyo;
class Plan;

struct EvalResult {
    EvalResult(double evaluation, const std::string& message);

    double evaluationScore;
    std::string message;
};

class AI {
public:
    AI(const std::string& name);

    std::string getName() const;

    void initialize(const Game&);

    void think(DropDecision& result, const Game&);
    void wnextAppeared(const Game&);
    void myRensaFinished(const Game&);
    void myOjamaDropped(const Game&);

    void enemyWNextAppeared(const Game&);
    void enemyGrounded(const Game&);

private:
    void decide(DropDecision&, const Game&);
    EvalResult eval(int currentFrameId, const Plan&, const Field& currentField) const;

private:
    std::string m_name;
    MyPlayerInfo m_myPlayerInfo;
    EnemyInfo m_enemyInfo;
};

#endif
