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
#include "core/field/field_pretty_printer.h"
#include "core/frame_data.h"
#include "core/kumipuyo.h"
#include "core/sequence_generator.h"
#include "core/state.h"

#include "evaluator.h"
#include "mayah_ai.h"
#include "util.h"

DEFINE_string(seq, "", "initial puyo sequence");

using namespace std;

class InteractiveAI : public MayahAI {
public:
    InteractiveAI(int argc, char* argv[]) : MayahAI(argc, argv) {}

    using MayahAI::gameWillBegin;
    using MayahAI::think;
    using MayahAI::enemyNext2Appeared;
    using MayahAI::reloadParameter;
    using MayahAI::makeMessageFrom;

    CollectedFeature makeCollectedFeature(int frameId, const CoreField& field, int numKeyPuyos, const Plan& plan) const
    {
        RefPlan refPlan(plan.field(), plan.decisions(), plan.rensaResult(), plan.numChigiri(), plan.initiatingFrames(), plan.lastDropFrames());
        return Evaluator(*featureParameter_, books_).evalWithCollectingFeature(refPlan, field, frameId, numKeyPuyos, gazer_);
    }

    Plan thinkPlanOnly(int frameId, const CoreField& field, const KumipuyoSeq& kumipuyoSeq,
                       int depth, const vector<Decision>& decisions) const
    {
        Plan result;
        Plan::iterateAvailablePlans(field, kumipuyoSeq, depth,
                                    [this, frameId, &field, &decisions, &result](const RefPlan& plan) {
            if (plan.decisions() != decisions)
                return;

            result = plan.toPlan();
        });

        return result;
    }

    void showMatchedBooks(const CoreField& field)
    {
        for (const auto& book : books_ ) {
            if (book.match(field)) {
                cout << book.toDebugString() << endl;
            }
        }
    }
};

// TODO(mayah): Implement with GUI!
int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    TsumoPossibility::initialize();

    InteractiveAI ai(argc, argv);

    CoreField field;
    KumipuyoSeq seq = generateSequence();

    if (FLAGS_seq != "") {
        for (size_t i = 0; i < FLAGS_seq.size(); ++i) {
            PuyoColor c = toPuyoColor(FLAGS_seq[i]);
            CHECK(isNormalColor(c));
            if (i % 2 == 0)
                seq.setAxis(i / 2, c);
            else
                seq.setChild(i / 2, c);
        }
    }

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

        // Waits for user enter.
        while (true) {
            FieldPrettyPrinter::print(field, seq.subsequence(i, 2));

            double t1 = now();
            Plan aiPlan = ai.thinkPlan(frameId, field, KumipuyoSeq { seq.get(i), seq.get(i + 1) },
                                       MayahAI::DEFAULT_DEPTH, MayahAI::DEFAULT_NUM_ITERATION);
            double t2 = now();
            if (aiPlan.decisions().empty())
                cout << "No decision";
            else
                cout << aiPlan.decision(0).toString() << "-" << aiPlan.decision(1).toString();
            cout << " time = " << ((t2 - t1) * 1000) << " [ms]" << endl;

            string str;
            cout << "command? ";
            getline(cin, str);

            if (str == "")
                break;
            if (str == "reload") {
                ai.reloadParameter();
                continue;
            }
            if (str == "s") {
                CollectedFeature cf = ai.makeCollectedFeature(frameId, field, MayahAI::DEFAULT_NUM_ITERATION, aiPlan);
                cout << cf.toString() << endl;
                cout << ai.makeMessageFrom(frameId, field, KumipuyoSeq { seq.get(i), seq.get(i + 1) },
                                           MayahAI::DEFAULT_NUM_ITERATION, aiPlan, (t2 - t1) * 1000);
                continue;
            }
            if (str == "book") {
                ai.showMatchedBooks(field);
                continue;
            }

            int x1, r1, x2, r2;
            if (sscanf(str.c_str(), "%d %d %d %d", &x1, &r1, &x2, &r2) == 4) {
                vector<Decision> decisions {
                    Decision(x1, r1),
                    Decision(x2, r2)
                };
                Plan plan = ai.thinkPlanOnly(frameId, field, KumipuyoSeq { seq.get(i), seq.get(i + 1) },
                                             MayahAI::DEFAULT_DEPTH, decisions);

                FieldPrettyPrinter::printMultipleFields(plan.field(), KumipuyoSeq { seq.get(i + 2), seq.get(i + 3) },
                                                        aiPlan.field(), KumipuyoSeq { seq.get(i + 2), seq.get(i + 3) });

                CollectedFeature mycf = ai.makeCollectedFeature(frameId, field, MayahAI::DEFAULT_NUM_ITERATION, plan);
                CollectedFeature aicf = ai.makeCollectedFeature(frameId, field, MayahAI::DEFAULT_NUM_ITERATION, aiPlan);
                cout << mycf.toStringComparingWith(aicf) << endl;
            }
        }

        Plan aiPlan = ai.thinkPlan(frameId, field, KumipuyoSeq { seq.get(i), seq.get(i + 1) },
                                   MayahAI::DEFAULT_DEPTH, MayahAI::DEFAULT_NUM_ITERATION);
        field.dropKumipuyo(aiPlan.decisions().front(), seq.get(i));
        field.simulate();
    }

    return 0;
}
