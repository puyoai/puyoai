#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/state.h"
#include "ai.h"
#include "game.h"
#include "puyo.h"
#include "puyo_possibility.h"

using namespace std;

static inline PuyoColor randomColor()
{
    return normalPuyoColorOf(rand() % 4);
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    TsumoPossibility::initialize();

    LOG(INFO) << "initialized" << endl;

    srand(0);
    vector<KumiPuyo> kumiPuyos;
    for (int i = 0; i < 50; ++i)
        kumiPuyos.push_back(KumiPuyo(randomColor(), randomColor()));

    Field enemyField(
        "500065"
        "400466"
        "545645"
        "456455"
        "545646"
        "545646"
        "564564"
        "456456"
        "456456"
        "456456"
        );

    Game game;
    game.playerStates[1].field = enemyField;
    game.playerStates[1].kumiPuyos.push_back(KumiPuyo(RED, RED));
    game.playerStates[1].kumiPuyos.push_back(KumiPuyo(RED, RED));
    game.playerStates[1].kumiPuyos.push_back(KumiPuyo(RED, RED));

    AI ai("solo");
    ai.initialize(game);

    ai.enemyWNextAppeared(game);
    ai.enemyGrounded(game);

    for (int i = 0; i < 40; ++i) {
        game.playerStates[0].kumiPuyos.clear();
        for (int j = 0; j < 3; ++j)
            game.playerStates[0].kumiPuyos.push_back(kumiPuyos[i + j]);

        DropDecision decision;
        ai.think(decision, game);
        ai.wnextAppeared(game);

        game.playerStates[0].field.dropKumiPuyo(decision.decision(), game.playerStates[0].kumiPuyos.front());
        BasicRensaResult info;
        game.playerStates[0].field.simulate(info);
        if (info.chains > 0) {
            cout << info.toString() << endl;
            cout << game.playerStates[0].field.debugOutput() << endl;
            break;
        }

        ai.myRensaFinished(game);
        cout << game.playerStates[0].field.debugOutput() << endl;

#if 0
        string s;
        getline(cin, s);
#endif
    }

    return 0;
}
