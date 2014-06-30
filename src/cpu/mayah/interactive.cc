#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/algorithm/puyo_possibility.h"
#include "core/algorithm/plan.h"
#include "core/algorithm/rensa_info.h"
#include "core/algorithm/rensa_detector.h"
#include "core/client/connector/drop_decision.h"
#include "core/field/core_field.h"
#include "core/frame_data.h"
#include "core/kumipuyo.h"
#include "core/sequence_generator.h"
#include "core/state.h"

#include "mayah_ai.h"
#include "util.h"

using namespace std;

class InteractiveAI : public MayahAI {
public:
    using MayahAI::gameWillBegin;
    using MayahAI::think;
    using MayahAI::enemyNext2Appeared;
};

// TODO(mayah): Implement with GUI!
int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    TsumoPossibility::initialize();

    {
        double t1 = now();
        CoreField f;
        KumipuyoSeq seq("RRBB");
        Plan::iterateAvailablePlans(f, seq, 4, [](const RefPlan&) {
            return;
        });
        double t2 = now();
        cout << (t2 - t1) << endl;
    }

    InteractiveAI ai;

    CoreField field;
    KumipuyoSeq seq = generateSequence();

    for (int i = 0; i + 1 < seq.size(); ++i) {
        int frameId = 2 + i; // frameId 1 will be used for initializing now. Let's avoid it.

        FrameData fd;
        fd.id = frameId;
        fd.valid = true;

        // Set up enemy field.
        // Make enemy will fire his large rensa.
        fd.playerFrameData[1].field = CoreField(
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
        fd.playerFrameData[1].kumipuyoSeq = KumipuyoSeq("666666");

        // Invoke Gazer.
        {
            double t1 = now();
            ai.enemyNext2Appeared(fd);
            double t2 = now();
            cout << "gazer time = " << (t2 - t1) << endl;
        }

        cout << field.debugOutput() << endl;
        cout << seq.get(i).toString() << " " << seq.get(i + 1).toString() << endl;

        // Waits for user enter.
        string str;
        cout << "enter? ";
        getline(cin, str);

        double t1 = now();
        DropDecision dropDecision = ai.think(frameId, field, KumipuyoSeq { seq.get(i), seq.get(i + 1) });
        double t2 = now();
        cout << dropDecision.decision().x << ' ' << dropDecision.decision().r << endl;
        cout << "time = " << (t2 - t1) << endl;

        field.dropKumipuyo(dropDecision.decision(), seq.get(i));
        field.simulate();
    }

    return 0;
}
