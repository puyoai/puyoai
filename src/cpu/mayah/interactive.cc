#include <fstream>
#include <iostream>
#include <stdlib.h>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/time.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/algorithm/plan.h"
#include "core/algorithm/rensa_info.h"
#include "core/algorithm/rensa_detector.h"
#include "core/field/core_field.h"
#include "core/field_pretty_printer.h"
#include "core/frame_request.h"
#include "core/kumipuyo.h"
#include "core/problem/problem.h"
#include "core/sequence_generator.h"
#include "core/state.h"

#include "evaluator.h"
#include "mayah_ai.h"

DEFINE_string(problem, "", "use problem");

using namespace std;

class InteractiveAI : public MayahAI {
public:
    InteractiveAI(int argc, char* argv[]) : MayahAI(argc, argv) {}

    using MayahAI::additionalThoughtInfo;
    using MayahAI::think;
    using MayahAI::reloadParameter;
    using MayahAI::makeMessageFrom;

    using MayahAI::gameWillBegin;
    using MayahAI::gameHasEnded;
    using MayahAI::enemyNext2Appeared;
    using MayahAI::enemyDecisionRequested;
    using MayahAI::enemyGrounded;

    CollectedFeature makeCollectedFeature(int frameId, const CoreField& field, int numKeyPuyos, const Plan& plan) const
    {
        FeatureScoreCollector sc(*featureParameter_);
        Evaluator<FeatureScoreCollector> evaluator(books_, &sc);

        RefPlan refPlan(plan.field(), plan.decisions(), plan.rensaResult(), plan.numChigiri(), plan.framesToInitiate(), plan.lastDropFrames());
        evaluator.collectScore(refPlan, field, frameId, numKeyPuyos, gazer_);

        return sc.toCollectedFeature();
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
            if (book.match(field).count) {
                cout << book.toDebugString() << endl;
            }
        }
    }
};

Problem makeProblem()
{
    KumipuyoSeq generated = generateSequence();

    Problem problem;
    if (!FLAGS_problem.empty()) {
        problem = Problem::readProblem(FLAGS_problem);

        // Add generated sequence after problem.
        problem.kumipuyoSeq[0].append(generated);
        problem.kumipuyoSeq[1].append(generated);
    } else {
        problem.kumipuyoSeq[0] = generated;
        problem.field[1] = CoreField(
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
        problem.kumipuyoSeq[1] = KumipuyoSeq("666666");
        problem.kumipuyoSeq[1].append(generated);
    }

    return problem;
}

// TODO(mayah): Implement with GUI!
int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    TsumoPossibility::initialize();

    InteractiveAI ai(argc, argv);

    Problem problem = makeProblem();

    FrameRequest req;
    req.frameId = 1;
    ai.gameWillBegin(req);

    req.frameId = 2;
    req.playerFrameRequest[0].field = problem.field[0];
    req.playerFrameRequest[1].field = problem.field[1];
    req.playerFrameRequest[0].kumipuyoSeq = problem.kumipuyoSeq[0];
    req.playerFrameRequest[1].kumipuyoSeq = problem.kumipuyoSeq[1];

    for (int i = 0; i < 50; ++i) {
        // frameId 1 will be used for initializing now. Let's avoid it.
        int frameId = 2 + i;
        req.frameId = frameId;

        // Call these callback for gazer.
        {
            double t1 = currentTime();
            ai.enemyDecisionRequested(req);
            ai.enemyNext2Appeared(req);
            ai.enemyGrounded(req);
            double t2 = currentTime();
            cout << "gazer time = " << (t2 - t1) << endl;
        }

        // Waits for user enter.
        while (true) {
            const PlainField& field = req.playerFrameRequest[0].field;
            const KumipuyoSeq& seq = req.playerFrameRequest[0].kumipuyoSeq;

            FieldPrettyPrinter::printMultipleFields(
                field, seq.subsequence(0, 2),
                req.playerFrameRequest[1].field,
                req.playerFrameRequest[1].kumipuyoSeq);

            double t1 = currentTime();
            ThoughtResult thoughtResult = ai.thinkPlan(frameId, field, seq.subsequence(0, 2), ai.additionalThoughtInfo(),
                                                       MayahAI::DEFAULT_DEPTH, MayahAI::DEFAULT_NUM_ITERATION);
            const Plan& aiPlan = thoughtResult.plan;

            double t2 = currentTime();
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
                cout << ai.makeMessageFrom(frameId, field, seq.subsequence(0, 2), MayahAI::DEFAULT_NUM_ITERATION,
                                           thoughtResult, (t2 - t1) * 1000);
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

        ThoughtResult thoughtResult = ai.thinkPlan(frameId,
                                                   req.playerFrameRequest[0].field,
                                                   req.playerFrameRequest[0].kumipuyoSeq.subsequence(0, 2),
                                                   ai.additionalThoughtInfo(),
                                                   MayahAI::DEFAULT_DEPTH, MayahAI::DEFAULT_NUM_ITERATION);
        {
            CoreField f = req.playerFrameRequest[0].field;
            f.dropKumipuyo(thoughtResult.plan.decisions().front(), req.playerFrameRequest[0].kumipuyoSeq.front());
            f.simulate();
            req.playerFrameRequest[0].field = f;
        }
        req.playerFrameRequest[0].kumipuyoSeq.dropFront();
    }

    return 0;
}
