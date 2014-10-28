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

class InteractiveAI : public DebuggableMayahAI {
public:
    InteractiveAI(int argc, char* argv[]) : DebuggableMayahAI(argc, argv) {}

    void showMatchedBooks(const CoreField& field)
    {
        for (const auto& book : books_) {
            if (book.match(field).count) {
                cout << book.toDebugString() << endl;
            }
        }
    }

    void removeNontokopuyoParameter()
    {
        featureParameter_->setValue(STRATEGY_ZENKESHI, 0);
        featureParameter_->setValue(STRATEGY_INITIAL_ZENKESHI, 0);
        featureParameter_->setValue(STRATEGY_TSUBUSHI, 0);
        featureParameter_->setValue(STRATEGY_IBARA, 0);
        featureParameter_->setValue(STRATEGY_SAISOKU, 0);
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
    ai.removeNontokopuyoParameter();

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
            ai.gaze(frameId, req.playerFrameRequest[1].field, req.playerFrameRequest[1].kumipuyoSeq);
            double t2 = currentTime();
            cout << "gazer time = " << (t2 - t1) << endl;
        }

        // Waits for user enter.
        while (true) {
            const PlainField& currentField = req.playerFrameRequest[0].field;
            const KumipuyoSeq& seq = req.playerFrameRequest[0].kumipuyoSeq;

            FieldPrettyPrinter::printMultipleFields(currentField, seq.subsequence(0, 2),
                                                    req.playerFrameRequest[1].field, req.playerFrameRequest[1].kumipuyoSeq);

            double t1 = currentTime();
            ThoughtResult aiThoughtResult = ai.thinkPlan(frameId, currentField, seq.subsequence(0, 2),
                                                         ai.myPlayerState(), ai.enemyPlayerState(),
                                                         MayahAI::DEFAULT_DEPTH, MayahAI::DEFAULT_NUM_ITERATION);
            const Plan& aiPlan = aiThoughtResult.plan;

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
            if (str == "book") {
                ai.showMatchedBooks(currentField);
                continue;
            }

            int x1, r1, x2, r2;
            int r = sscanf(str.c_str(), "%d %d %d %d", &x1, &r1, &x2, &r2);
            if (r == 2 || r == 4) {
                vector<Decision> decisions;
                if (r == 2) {
                    Decision d1(x1, r1);
                    if (!d1.isValid())
                        continue;
                    decisions.push_back(d1);
                } else if (r == 4) {
                    Decision d1(x1, r1);
                    Decision d2(x2, r2);
                    if (!d1.isValid() || !d2.isValid())
                        continue;
                    decisions.push_back(d1);
                    decisions.push_back(d2);
                } else {
                    continue;
                }

                ThoughtResult myThoughtResult = ai.thinkPlan(frameId, currentField, KumipuyoSeq { seq.get(0), seq.get(1) },
                                                             ai.myPlayerState(), ai.enemyPlayerState(),
                                                             MayahAI::DEFAULT_DEPTH, MayahAI::DEFAULT_NUM_ITERATION, &decisions);

                FieldPrettyPrinter::printMultipleFields(myThoughtResult.plan.field(), KumipuyoSeq { seq.get(2), seq.get(3) },
                                                        aiThoughtResult.plan.field(), KumipuyoSeq { seq.get(2), seq.get(3) });

                const PreEvalResult preEvalResult = ai.preEval(currentField);
                CollectedFeature mycf = ai.evalWithCollectingFeature(RefPlan(myThoughtResult.plan), currentField, frameId, MayahAI::DEFAULT_NUM_ITERATION,
                                                                     ai.myPlayerState(), ai.enemyPlayerState(), preEvalResult, myThoughtResult.midEvalResult,
                                                                     ai.gazer().gazeResult());
                CollectedFeature aicf = ai.evalWithCollectingFeature(RefPlan(aiThoughtResult.plan), currentField, frameId, MayahAI::DEFAULT_NUM_ITERATION,
                                                                     ai.myPlayerState(), ai.enemyPlayerState(), preEvalResult, aiThoughtResult.midEvalResult,
                                                                     ai.gazer().gazeResult());
                cout << mycf.toStringComparingWith(aicf) << endl;
            }
        }

        ThoughtResult thoughtResult = ai.thinkPlan(frameId,
                                                   req.playerFrameRequest[0].field,
                                                   req.playerFrameRequest[0].kumipuyoSeq.subsequence(0, 2),
                                                   PlayerState(), PlayerState(),
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
