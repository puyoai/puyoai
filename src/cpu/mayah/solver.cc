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
#include "ai.h"

using namespace std;

void readField(FrameData& frameData)
{
    string str;
    string leftField, rightField;

    while (getline(cin, str) && str != "END") {
        istringstream ss(str);
        string l, r; ss >> l >> r;
        leftField += l;
        rightField += r;
    }

    frameData.playerFrameData[0].field = Field(leftField);
    frameData.playerFrameData[1].field = Field(rightField);
}

void readNext(KumipuyoSeq kumipuyoSeq[2])
{
    string str;
    getline(cin, str);

    istringstream ss(str);
    string l, r; ss >> l >> r;
    kumipuyoSeq[0] = KumipuyoSeq(l);
    kumipuyoSeq[1] = KumipuyoSeq(r);
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

void readProblem(FrameData& frameData, Decision& enemyFirstDecision,
                 KumipuyoSeq kumipuyoSeq[2], map<pair<Decision, Decision>, int>& score)
{
    frameData.id = 1;
    frameData.state = STATE_YOU_CAN_PLAY | (STATE_YOU_CAN_PLAY << 1);

    frameData.playerFrameData[0].score = 0;
    frameData.playerFrameData[0].ojama = 0;
    frameData.playerFrameData[1].score = 0;
    frameData.playerFrameData[1].ojama = 0;

    string str;
    while (getline(cin, str)) {
        if (str == "FIELD")
            readField(frameData);
        else if (str == "NEXT")
            readNext(kumipuyoSeq);
        else if (str == "ENEMY")
            readEnemyHand(enemyFirstDecision);
        else if (str == "ANSWER")
            readAnswer(score);
    }

    cerr << frameData.playerFrameData[0].field.debugOutput() << endl;
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    TsumoPossibility::initialize();

    LOG(INFO) << "initialized" << endl;

    AI ai("server-log");

    FrameData frameData;
    Decision enemyFirstDecision;
    KumipuyoSeq kumipuyos[2];
    map<pair<Decision, Decision>, int> score;
    readProblem(frameData, enemyFirstDecision, kumipuyos, score);

    frameData.playerFrameData[0].kumipuyoSeq = kumipuyos[0];
    frameData.playerFrameData[1].kumipuyoSeq = kumipuyos[1];

    frameData.playerFrameData[0].kumipuyoSeq.resize(3);
    frameData.playerFrameData[1].kumipuyoSeq.resize(3);

    ai.initialize(frameData);

    DropDecision firstDecision = ai.think(frameData);
    cerr << "your decision (1): " << firstDecision.decision().x << ' ' << firstDecision.decision().r << endl;

    frameData.playerFrameData[0].field.dropKumipuyo(firstDecision.decision(), frameData.playerFrameData[0].kumipuyoSeq.front());
    frameData.playerFrameData[1].field.dropKumipuyo(enemyFirstDecision, frameData.playerFrameData[1].kumipuyoSeq.front());
    frameData.playerFrameData[0].kumipuyoSeq = kumipuyos[0].subsequence(1, 3);
    frameData.playerFrameData[1].kumipuyoSeq = kumipuyos[1].subsequence(1, 3);

    ai.enemyGrounded(frameData);
    ai.enemyWNextAppeared(frameData);

    DropDecision secondDecision = ai.think(frameData);
    cerr << "your decision (2): " << secondDecision.decision().x << ' ' << secondDecision.decision().r << endl;

    frameData.playerFrameData[0].field.dropKumipuyo(secondDecision.decision(), frameData.playerFrameData[0].kumipuyoSeq.front());
    cerr << frameData.playerFrameData[0].field.debugOutput() << endl;

    map<pair<Decision, Decision>, int>::iterator it = score.find(make_pair(firstDecision.decision(), secondDecision.decision()));

    if (it == score.end())
        cout << "score = " << 0 << endl;
    else
        cout << "score = " << it->second << endl;

    return 0;
}
