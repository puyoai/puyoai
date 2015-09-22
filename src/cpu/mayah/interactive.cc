#include <stdlib.h>

#include <fstream>
#include <iostream>
#include <tuple>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/strings.h"
#include "base/time.h"
#include "core/plan/plan.h"
#include "core/rensa/rensa_detector.h"
#include "core/core_field.h"
#include "core/field_pretty_printer.h"
#include "core/frame_request.h"
#include "core/kumipuyo.h"
#include "core/kumipuyo_seq_generator.h"
#include "core/probability/puyo_set_probability.h"
#include "solver/problem.h"

#include "evaluator.h"
#include "mayah_ai.h"

DEFINE_string(problem, "", "use problem");
DEFINE_bool(tokopuyo, false, "Use tokopuyo parameter");

using namespace std;

class InteractiveAI : public DebuggableMayahAI {
public:
    InteractiveAI(int argc, char* argv[]) : DebuggableMayahAI(argc, argv) {}
};

Problem makeProblem()
{
    KumipuyoSeq generated = KumipuyoSeqGenerator::generateACPuyo2Sequence();
    LOG(INFO) << "seq=" << generated.toString();

    Problem problem;
    if (!FLAGS_problem.empty()) {
        problem = Problem::readProblem(FLAGS_problem);

        // Add generated sequence after problem.
        problem.myState.seq.append(generated);
        problem.enemyState.seq.append(generated);
    } else {
        problem.myState.seq = generated;
        problem.enemyState.field = CoreField(
            "5...65"
            "4...66"
            "545645"
            "456455"
            "545646"
            "545646"
            "564564"
            "456456"
            "456456"
            "456456");
        problem.enemyState.seq = KumipuyoSeq("666666");
        problem.enemyState.seq.append(generated);
    }

    return problem;
}

// TODO(mayah): Implement with GUI!
int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    InteractiveAI ai(argc, argv);
    if (FLAGS_tokopuyo) {
        ai.removeNontokopuyoParameter();
        cout << "CAUTION --- tokopuyo parameter is used. --- CAUTION" << endl;
    } else {
        cout << "Non tokopuyo parameter is used." << endl;
    }

    Problem problem = makeProblem();

    FrameRequest req;
    req.frameId = 1;
    ai.gameWillBegin(req);

    req.frameId = 2;
    req.playerFrameRequest[0].field = problem.myState.field.toPlainField();
    req.playerFrameRequest[1].field = problem.enemyState.field.toPlainField();
    req.playerFrameRequest[0].kumipuyoSeq = problem.myState.seq;
    req.playerFrameRequest[1].kumipuyoSeq = problem.enemyState.seq;

    ai.mutableMyPlayerState()->field = problem.myState.field;
    ai.mutableMyPlayerState()->seq = problem.enemyState.seq;
    ai.mutableEnemyPlayerState()->field = problem.enemyState.field;
    ai.mutableEnemyPlayerState()->seq = problem.enemyState.seq;

    for (int i = 0; i < 50; ++i) {
        // frameId 1 will be used for initializing now. Let's avoid it.
        const int frameId = 2 + i;
        req.frameId = frameId;

        // Call these callback for gazer.
        {
            double t1 = currentTime();
            ai.gaze(frameId, CoreField(req.playerFrameRequest[1].field), req.playerFrameRequest[1].kumipuyoSeq);
            double t2 = currentTime();
            cout << "gazer time = " << (t2 - t1) << endl;
        }

        // Waits for user enter.
        while (true) {
            CoreField currentField(req.playerFrameRequest[0].field);
            const KumipuyoSeq& seq = req.playerFrameRequest[0].kumipuyoSeq;

            FieldPrettyPrinter::printMultipleFields(
                { currentField.toPlainField(), req.playerFrameRequest[1].field },
                { seq.subsequence(0, 2), req.playerFrameRequest[1].kumipuyoSeq });
            double t1 = currentTime();
            ThoughtResult aiThoughtResult = ai.thinkPlan(frameId, currentField, seq.subsequence(0, 2),
                                                         ai.myPlayerState(), ai.enemyPlayerState(),
                                                         MayahAI::DEFAULT_DEPTH, MayahAI::DEFAULT_NUM_ITERATION);
            const Plan& aiPlan = aiThoughtResult.plan;

            double t2 = currentTime();
            if (aiPlan.decisions().empty())
                cout << "No decision";
            else {
                for (size_t i = 0; i < aiPlan.decisions().size(); ++i) {
                    if (i)
                        cout << "-";
                    cout << aiPlan.decision(i).toString();
                }
            }
            cout << " time = " << ((t2 - t1) * 1000) << " [ms]" << endl;

            string str;
            cout << "command? ";
            getline(cin, str);

            if (str == "")
                break;
            if (str == "reload") {
                ai.loadEvaluationParameter();
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

                ThoughtResult myThoughtResult = ai.thinkPlan(frameId, currentField,  seq.subsequence(0, 2),
                                                             ai.myPlayerState(), ai.enemyPlayerState(),
                                                             MayahAI::DEFAULT_DEPTH, MayahAI::DEFAULT_NUM_ITERATION, false, &decisions);

                const PreEvalResult preEvalResult = ai.preEval(currentField);
                CollectedFeatureCoefScore mycf = ai.evalWithCollectingFeature(
                    RefPlan(myThoughtResult.plan),
                    seq.subsequence(0, 2).subsequence(myThoughtResult.plan.decisions().size()),
                    frameId, MayahAI::DEFAULT_NUM_ITERATION,
                    ai.myPlayerState(), ai.enemyPlayerState(), preEvalResult, myThoughtResult.midEvalResult, false,
                    ai.gazer().gazeResult());
                CollectedFeatureCoefScore aicf = ai.evalWithCollectingFeature(
                    RefPlan(aiThoughtResult.plan),
                    seq.subsequence(0, 2).subsequence(aiThoughtResult.plan.decisions().size()),
                    frameId, MayahAI::DEFAULT_NUM_ITERATION,
                    ai.myPlayerState(), ai.enemyPlayerState(), preEvalResult, aiThoughtResult.midEvalResult, false,
                    ai.gazer().gazeResult());

                CoreField myTargetField(myThoughtResult.plan.field());
                myTargetField.dropPuyoList(mycf.mainRensaScore().puyosToComplement);

                CoreField aiTargetField(aiThoughtResult.plan.field());
                aiTargetField.dropPuyoList(aicf.mainRensaScore().puyosToComplement);

                KumipuyoSeq seqToShow { seq.get(2), seq.get(3) };
                FieldPrettyPrinter::printMultipleFields(
                    { myThoughtResult.plan.field().toPlainField(), aiThoughtResult.plan.field().toPlainField(), myTargetField.toPlainField(), aiTargetField.toPlainField() },
                    { seqToShow, seqToShow, KumipuyoSeq(), KumipuyoSeq() });

                cout << CollectedFeatureCoefScore::scoreComparisionString(mycf, aicf, ai.evaluationParameterMap()) << endl;
                cout << "MY: " << myThoughtResult.message << endl;
                cout << "AI: " << aiThoughtResult.message << endl;
            }
        }

        ThoughtResult thoughtResult = ai.thinkPlan(frameId,
                                                   CoreField(req.playerFrameRequest[0].field),
                                                   req.playerFrameRequest[0].kumipuyoSeq.subsequence(0, 2),
                                                   ai.myPlayerState(),
                                                   ai.enemyPlayerState(),
                                                   MayahAI::DEFAULT_DEPTH, MayahAI::DEFAULT_NUM_ITERATION);
        {
            CoreField f(req.playerFrameRequest[0].field);
            f.dropKumipuyo(thoughtResult.plan.decisions().front(), req.playerFrameRequest[0].kumipuyoSeq.front());
            f.simulate();
            req.playerFrameRequest[0].field = f.toPlainField();
        }
        req.playerFrameRequest[0].kumipuyoSeq.dropFront();
    }

    return 0;
}
