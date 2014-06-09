#include <iostream>
#include <fstream>
#include <stdlib.h>

#include "core/algorithm/rensa_info.h"
#include "core/algorithm/rensa_detector.h"
#include "core/client/connector/drop_decision.h"
#include "core/frame_data.h"
#include "core/kumipuyo.h"
#include "core/state.h"
#include "evaluation_feature_collector.h"
#include "field.h"
#include "player_info.h"

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
        "456456");

    AI ai("interactive");

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
        cout << myField.debugOutput() << endl;

        FrameData frameData;
        frameData.id = currentFrameId;
        frameData.state = STATE_YOU_CAN_PLAY | (STATE_YOU_CAN_PLAY << 1);
        frameData.playerFrameData[0].score = 0;
        frameData.playerFrameData[0].ojama = 0;
        frameData.playerFrameData[0].field = myField;
        frameData.playerFrameData[0].kumipuyoSeq = KumipuyoSeq(nextPuyos);
        frameData.playerFrameData[1].score = 0;
        frameData.playerFrameData[1].ojama = 0;
        frameData.playerFrameData[1].field = enemyField;
        frameData.playerFrameData[1].kumipuyoSeq = KumipuyoSeq("666666");

        vector<FeasibleRensaInfo> feasibleRensaInfos =
            RensaDetector::findFeasibleRensas(enemyField, frameData.enemyPlayerFrameData().kumipuyoSeq);

        for (vector<FeasibleRensaInfo>::iterator it = feasibleRensaInfos.begin(); it != feasibleRensaInfos.end(); ++it) {
            cout << "score  = " << it->basicRensaResult.score
                 << "chains = " << it->basicRensaResult.chains
                 << "frames = " << it->basicRensaResult.frames
                 << "initi  = " << it->initiatingFrames << endl;
        }

        string str;
        cout << "enter? ";
        getline(cin, str);

        DropDecision dropDecision = ai.think(frameData);
        cout << dropDecision.decision().x << ' ' << dropDecision.decision().r << endl;

        myField.dropKumipuyo(dropDecision.decision(), frameData.myPlayerFrameData().kumipuyoSeq.front());

        BasicRensaResult rensaInfo = myField.simulate();

        cout << rensaInfo.chains << ' ' << rensaInfo.score << ' ' << rensaInfo.frames << endl;
        cout << (myField.color(dropDecision.decision().x, 12) + '0') << endl;

        nextPuyos = nextPuyos.substr(2);
    }


    return 0;
}

