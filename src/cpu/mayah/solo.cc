#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/algorithm/puyo_possibility.h"
#include "core/client/connector/drop_decision.h"
#include "core/frame_data.h"
#include "core/kumipuyo.h"
#include "core/state.h"
#include "cpu/mayah/ai.h"

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
    vector<Kumipuyo> kumipuyos;
    for (int i = 0; i < 50; ++i)
        kumipuyos.push_back(Kumipuyo(randomColor(), randomColor()));

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

    FrameData frameData;
    frameData.playerFrameData[1].field = enemyField;
    frameData.playerFrameData[1].kumipuyoSeq = KumipuyoSeq("RRRRRR");

    AI ai("solo");
    ai.initialize(frameData);

    ai.enemyWNextAppeared(frameData);
    ai.enemyGrounded(frameData);

    for (int i = 0; i < 40; ++i) {
        frameData.playerFrameData[0].kumipuyoSeq = KumipuyoSeq {
            kumipuyos[i], kumipuyos[i + 1], kumipuyos[i + 2]
        };

        DropDecision decision = ai.think(frameData);

        frameData.playerFrameData[0].field.dropKumipuyo(decision.decision(), frameData.playerFrameData[0].kumipuyoSeq.front());
        BasicRensaResult info = frameData.playerFrameData[0].field.simulate();
        if (info.chains > 0) {
            cout << info.toString() << endl;
            cout << frameData.playerFrameData[0].field.debugOutput() << endl;
            break;
        }

        ai.myRensaFinished(frameData);
        cout << frameData.playerFrameData[0].field.debugOutput() << endl;

#if 0
        string s;
        getline(cin, s);
#endif
    }

    return 0;
}
