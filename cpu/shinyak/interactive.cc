#include <iostream>
#include <fstream>
#include <stdlib.h>

#include "../../core/state.h"
#include "ai.h"
#include "field.h"
#include "field_evaluator.h"
#include "game.h"
#include "player_info.h"
#include "rensainfo.h"

using namespace std;

int main(void)
{
    srand(2);

    Field myField;
    Field enemyField(
        "500065"
        "400066"
        "545645"
        "456455"
        "545646"
        "545646"
        "564564"
        "456456"
        "456456"
        "456456"
        );
    AI ai;
    ofstream log("hoge.txt");
    Decision decision;
    
    string nextPuyos;
    for (int i = 0; i < 4; ++i) {
        char r = (rand() % 4) + '4';
        nextPuyos += r;
    }

    int currentFrameId = 1;
    while (true) {
        for (int i = 0; i < 2; ++i) {
            char r = (rand() % 4) + '4';
            nextPuyos += r;
        }

        cout << "NextPuyo = " << nextPuyos << endl;
        cout << myField.getDebugOutput() << endl;

        Game game;
        game.id = currentFrameId;
        game.state = STATE_YOU_CAN_PLAY | (STATE_YOU_CAN_PLAY << 1);
        game.playerStates[0].score = 0;
        game.playerStates[0].ojama = 0;
        game.playerStates[0].field = myField;
        setKumiPuyo(nextPuyos, game.playerStates[0].kumiPuyos);        
        game.playerStates[1].score = 0;
        game.playerStates[1].ojama = 0;
        game.playerStates[1].field = enemyField;
        setKumiPuyo("666666", game.playerStates[1].kumiPuyos);        

        vector<FeasibleRensaInfo> feasibleRensaInfos;
        enemyField.findFeasibleRensas(feasibleRensaInfos, 3, game.enemyPlayerState().kumiPuyos);
        for (vector<FeasibleRensaInfo>::iterator it = feasibleRensaInfos.begin(); it != feasibleRensaInfos.end(); ++it) {
            cout << "score  = " << it->rensaInfo.score
                 << "chains = " << it->rensaInfo.chains
                 << "frames = " << it->rensaInfo.frames
                 << "initi  = " << it->initiatingFrames << endl;
        }

        string str;
        cout << "enter? ";
        getline(cin, str);

        ai.think(decision, game, log);
        cout << decision.x << ' ' << decision.r << endl;

        myField.dropKumiPuyo(decision, game.myPlayerState().kumiPuyos[0]);

        BasicRensaInfo rensaInfo;
        myField.simulate(rensaInfo);

        cout << rensaInfo.chains << ' ' << rensaInfo.score << ' ' << rensaInfo.frames << endl;
        cout << (myField.color(decision.x, 12) + '0') << endl;

        nextPuyos = nextPuyos.substr(2);
    }


    return 0;
}

