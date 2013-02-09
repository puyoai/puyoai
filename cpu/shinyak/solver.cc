#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include "../../core/state.h"
#include "ai.h"
#include "game.h"
#include "puyo_possibility.h"

using namespace std;

void readField(Game& game)
{
    string str;
    string leftField, rightField;

    while (getline(cin, str) && str != "END") {
        istringstream ss(str);
        string l, r; ss >> l >> r;
        leftField += l;
        rightField += r;
    }

    game.playerStates[0].field = Field(leftField);
    game.playerStates[1].field = Field(rightField);
}

void readNext(vector<KumiPuyo> kumiPuyos[2])
{
    string str;
    getline(cin, str);

    istringstream ss(str);
    string l, r; ss >> l >> r;
    setKumiPuyo(l, kumiPuyos[0]);
    setKumiPuyo(r, kumiPuyos[1]);
    cerr << "CURRENT/NEXT/NEXTNEXT = " << l << endl;

    getline(cin, str); // skip END
}

void readEnemyHand(Decision& enemyFirstDecision)
{
    string str;
    getline(cin, str);

    istringstream ss(str);
    int x, r;
    ss >> x >> r;
    enemyFirstDecision = Decision(x, r);

    getline(cin, str); // skip END
}

void readAnswer(map<pair<Decision, Decision>, int>& score)
{
    string str;
    while (getline(cin, str) && str != "END") {
        int s, x1, r1, x2, r2;
        istringstream ss(str);
        ss >> s >> x1 >> r1 >> x2 >> r2;
        score[make_pair(Decision(x1, r1), Decision(x2, r2))] = s;
    }
}

void readProblem(Game& game, Decision& enemyFirstDecision, 
                 vector<KumiPuyo> kumiPuyos[2], map<pair<Decision, Decision>, int>& score)
{
    game.id = 1;
    game.state = STATE_YOU_CAN_PLAY | (STATE_YOU_CAN_PLAY << 1);

    game.playerStates[0].score = 0;
    game.playerStates[0].ojama = 0;
    game.playerStates[1].score = 0;
    game.playerStates[1].ojama = 0;

    string str;
    while (getline(cin, str)) {
        if (str == "FIELD")
            readField(game);
        else if (str == "NEXT")
            readNext(kumiPuyos);
        else if (str == "ENEMY")
            readEnemyHand(enemyFirstDecision);
        else if (str == "ANSWER")
            readAnswer(score);
    }

    cerr << game.playerStates[0].field.getDebugOutput() << endl;
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    TsumoPossibility::initialize();

    LOG(INFO) << "initialized" << endl;

    AI ai("server-log");

    Game game;
    Decision enemyFirstDecision;
    vector<KumiPuyo> kumiPuyos[2];
    map<pair<Decision, Decision>, int> score;
    readProblem(game, enemyFirstDecision, kumiPuyos, score);

    game.playerStates[0].kumiPuyos.clear();
    game.playerStates[1].kumiPuyos.clear();
    for (int i = 0; i < 3; ++i) {
        game.playerStates[0].kumiPuyos.push_back(kumiPuyos[0][i]);
        game.playerStates[1].kumiPuyos.push_back(kumiPuyos[1][i]);
    }

    ai.initialize(game);

    Decision firstDecision;
    ai.think(firstDecision, game);
    cerr << "your decision (1): " << firstDecision.x << ' ' << firstDecision.r << endl;

    game.playerStates[0].field.dropKumiPuyo(firstDecision, game.playerStates[0].kumiPuyos[0]);
    game.playerStates[1].field.dropKumiPuyo(enemyFirstDecision, game.playerStates[1].kumiPuyos[0]);
    game.playerStates[0].kumiPuyos.clear();
    game.playerStates[1].kumiPuyos.clear();
    for (int i = 0; i < 3; ++i) {
        game.playerStates[0].kumiPuyos.push_back(kumiPuyos[0][i + 1]);
        game.playerStates[1].kumiPuyos.push_back(kumiPuyos[1][i + 1]);
    }

    ai.enemyGrounded(game);
    ai.enemyWNextAppeared(game);

    Decision secondDecision;
    ai.think(secondDecision, game);
    cerr << "your decision (2): " << secondDecision.x << ' ' << secondDecision.r << endl;

    game.playerStates[0].field.dropKumiPuyo(secondDecision, game.playerStates[0].kumiPuyos[0]);
    cerr << game.playerStates[0].field.getDebugOutput() << endl;

    map<pair<Decision, Decision>, int>::iterator it = score.find(make_pair(firstDecision, secondDecision));

    if (it == score.end())
        cout << "score = " << 0 << endl;
    else
        cout << "score = " << it->second << endl;

    return 0;
}
