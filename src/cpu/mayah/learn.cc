#include <algorithm>
#include <iostream>
#include <math.h>
#include <set>
#include <stdio.h>
#include <string>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/algorithm/plan.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/field/core_field.h"
#include "core/puyo_color.h"
#include "core/sequence_generator.h"

#include "evaluator.h"
#include "feature_parameter.h"
#include "gazer.h"

using namespace std;

// const double LEARNING_COEF = 1 / 1000.0;
const double WINDOW_SIZE = 65536;
const double PROGRESSION = 7 * 256;

// The derivation of sigmoid
static double dsigmoid(double x)
{
    const double delta = WINDOW_SIZE / PROGRESSION;

    if (x < -WINDOW_SIZE)
        return 0.0;
    if (WINDOW_SIZE < x)
        return 0.0;

    double t = exp(-x / delta);
    return t / (delta * (1 + t) * (1 + t));
}

void updateFeature(FeatureParameter* parameter,
                   const CollectedFeature& currentFeature,
                   const CollectedFeature& teacherFeature)
{
    const double D = dsigmoid(currentFeature.score - teacherFeature.score);

    for (int i = 0; i < SIZE_OF_EVALUATION_FEATURE_KEY; ++i) {
        EvaluationFeatureKey key = toEvaluationFeatureKey(i);

        // Constraint: We don't change TOTAL_FRAMES parameter.
        if (key == TOTAL_FRAMES)
            continue;

        double curV = currentFeature.collectedFeatures.count(key) ? currentFeature.collectedFeatures.find(key)->second : 0;
        double teaV = teacherFeature.collectedFeatures.count(key) ? teacherFeature.collectedFeatures.find(key)->second : 0;
        double curScore = parameter->score(key, curV);
        double teaScore = parameter->score(key, teaV);
        double dT = D * (curScore - teaScore);
        cout << "learning: " << toString(key) << ' '
             << "current: val=" << curV << " score=" << curScore << "  "
             << "teacher: val=" << teaV << " score=" << teaScore << "  "
             << dT << endl;
        parameter->addValue(key, -dT);
    }

    for (int i = 0; i < SIZE_OF_EVALUATION_SPARSE_FEATURE_KEY; ++i) {
        EvaluationSparseFeatureKey key = toEvaluationSparseFeatureKey(i);
        map<int, pair<int, int>> scores;
        for (const auto& entry : currentFeature.collectedSparseFeatures) {
            for (int v : entry.second)
                scores[v].first += 1;
        }
        for (const auto& entry : teacherFeature.collectedSparseFeatures) {
            for (int v : entry.second)
                scores[v].second += 1;
        }

        for (const auto& entry : scores) {
            double curV = entry.second.first;
            double teaV = entry.second.second;
            double curScore = parameter->score(key, entry.first) * curV;
            double teaScore = parameter->score(key, entry.first) * teaV;
            double dT = D * (curScore - teaScore);
            parameter->addValue(key, entry.first, -dT);
        }
    }
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    TsumoPossibility::initialize();

    FeatureParameter parameter("feature.txt");
    Gazer gazer;

    CoreField field;
    KumipuyoSeq kumipuyoSeq = generateSequence();

    for (int i = 0; i + 2 < kumipuyoSeq.size(); ++i) {
        KumipuyoSeq seq = kumipuyoSeq.subsequence(i, 2);

        // Collects features of our decision
        map<pair<Decision, Decision>, CollectedFeature> collectedFeatures;
        Plan::iterateAvailablePlans(field, seq, 2, [&parameter, &gazer, &collectedFeatures](const RefPlan& plan) {
            CollectedFeature f = Evaluator(parameter).evalWithCollectingFeature(plan, 1, gazer);
            if (plan.decisions().size() == 1)
                collectedFeatures[make_pair(plan.decision(0), Decision())] = f;
            else if (plan.decisions().size() >= 2)
                collectedFeatures[make_pair(plan.decision(0), plan.decision(1))] = f;
        });

        // Lists teacher decision.
        pair<Decision, Decision> teacherDecision;
        while (!teacherDecision.first.isValid()) {
            cout << field.debugOutput() << endl
                 << seq.toString() << endl;
            cout << "Decision x1 r1 x2 r2 ? " << flush;
            string str;
            getline(cin, str);
            if (str == "q" || str == "quit" || str == "exit")
                break;

            int x1, r1, x2, r2;
            if (sscanf(str.c_str(), "%d %d %d %d", &x1, &r1, &x2, &r2) != 4)
                continue;

            if (!Decision(x2, r2).isValid()) {
                x2 = 0;
                r2 = 0;
            }
            teacherDecision = make_pair(Decision(x1, r1), Decision(x2, r2));
        }

        if (!teacherDecision.first.isValid())
            break;

        field.dropKumipuyo(teacherDecision.first, seq.get(0));
    }

    CHECK(parameter.save("learned.txt"));
    return 0;
}
