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

DEFINE_bool(interactive, false, "use interactive learning");
DECLARE_string(feature);

// const double LEARNING_COEF = 1 / 1000.0;
// TODO(mayah): Add L2-normalization.
const double WINDOW_SIZE = 256;
const double PROGRESSION =  32;

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
    cout << "learning: D = " << D
         << " current = " << currentFeature.score
         << " teacher = " << teacherFeature.score
         << endl;

    for (int i = 0; i < SIZE_OF_EVALUATION_FEATURE_KEY; ++i) {
        EvaluationFeatureKey key = toEvaluationFeatureKey(i);

        // Constraint: We don't change TOTAL_FRAMES parameter.
        if (key == TOTAL_FRAMES)
            continue;

        double curV = currentFeature.collectedFeatures.count(key) ? currentFeature.collectedFeatures.find(key)->second : 0;
        double teaV = teacherFeature.collectedFeatures.count(key) ? teacherFeature.collectedFeatures.find(key)->second : 0;
        if (curV == teaV)
            continue;

        double curScore = parameter->score(key, curV);
        double teaScore = parameter->score(key, teaV);
        double dT = D * (curV - teaV);
        cout << "learning: " << toString(key) << ' '
             << "current: val=" << curV << " score=" << curScore << "  "
             << "teacher: val=" << teaV << " score=" << teaScore << "  "
             << dT << endl;
        parameter->addValue(key, -dT);
    }

    for (int i = 0; i < SIZE_OF_EVALUATION_SPARSE_FEATURE_KEY; ++i) {
        EvaluationSparseFeatureKey key = toEvaluationSparseFeatureKey(i);
        map<int, pair<int, int>> scores;
        if (currentFeature.collectedSparseFeatures.count(key)) {
            for (int v : currentFeature.collectedSparseFeatures.find(key)->second)
                scores[v].first += 1;
        }
        if (teacherFeature.collectedSparseFeatures.count(key)) {
            for (int v : teacherFeature.collectedSparseFeatures.find(key)->second)
                scores[v].second += 1;
        }

        for (const auto& entry : scores) {
            int curV = entry.second.first;
            int teaV = entry.second.second;
            if (curV == teaV)
                continue;
            double curScore = parameter->score(key, entry.first, curV);
            double teaScore = parameter->score(key, entry.first, teaV);
            double dT = D * (curV - teaV);
            cout << "learning: " << toString(key) << ' ' << entry.first << " "
                 << "current: val=" << curV << " score=" << curScore << "  "
                 << "teacher: val=" << teaV << " score=" << teaScore << "  "
                 << dT << endl;
            parameter->addValue(key, entry.first, -dT);
        }
    }
}

void learnWithInteractive()
{
    FeatureParameter parameter(FLAGS_feature);
    cout << parameter.toString() << endl;
    Gazer gazer;

    CoreField field;
    KumipuyoSeq kumipuyoSeq = generateSequence();

    for (int i = 0; i + 2 < kumipuyoSeq.size(); ++i) {
        KumipuyoSeq seq = kumipuyoSeq.subsequence(i, 2);

        // Collects features of our decision
        map<pair<Decision, Decision>, CollectedFeature> collectedFeatures;
        double currentScore = -1000000;
        pair<Decision, Decision> currentDecision;
        Plan::iterateAvailablePlans(field, seq, 2, [&](const RefPlan& plan) {
            CollectedFeature f = Evaluator(parameter).evalWithCollectingFeature(plan, 1, gazer);
            pair<Decision, Decision> pd;
            if (plan.decisions().size() == 1)
                pd = make_pair(plan.decision(0), Decision());
            else
                pd = make_pair(plan.decision(0), plan.decision(1));
            collectedFeatures[pd] = f;
            if (currentScore < f.score) {
                currentScore = f.score;
                currentDecision = pd;
            }
        });

        // Lists teacher decision.
        pair<Decision, Decision> teacherDecision;
        while (!teacherDecision.first.isValid()) {
            cout << field.debugOutput() << endl
                 << seq.toString() << endl;
            cout << "our decision : "
                 << currentDecision.first.x << ' '
                 << currentDecision.first.r << ' '
                 << currentDecision.second.x << ' '
                 << currentDecision.second.r << ' '
                 << "score=" << currentScore << endl;

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

        const CollectedFeature* currentFeature = nullptr;
        const CollectedFeature* teacherFeature = nullptr;
        for (const auto& entry : collectedFeatures) {
            if (!currentFeature)
                currentFeature = &entry.second;
            else if (currentFeature->score < entry.second.score)
                currentFeature = &entry.second;

            if (entry.first == teacherDecision)
                teacherFeature = &entry.second;
        }

        field.dropKumipuyo(teacherDecision.first, seq.get(0));
        field.simulate();

        if (currentFeature == nullptr) {
            cout << "currentFeature is null.";
            continue;
        }

        if (teacherFeature == nullptr) {
            cout << "teacherFeature is null.";
            continue;
        }

        if (currentFeature != teacherFeature)
            updateFeature(&parameter, *currentFeature, *teacherFeature);
    }

    CHECK(parameter.save("learned.txt"));
}

struct FrameInput {
    FrameInput(const CoreField& f0, const KumipuyoSeq& s, const CoreField& f1) :
        field(f0),
        seq(s),
        fieldAfter(f1)
    {
    }

    CoreField field;
    KumipuyoSeq seq;
    CoreField fieldAfter;
};

vector<FrameInput> readUntilGameEnd()
{
    vector<FrameInput> inputs;
    string left, sequence, right;
    while (cin >> left >> sequence >> right) {
        if (left == "===")
            break;

        cout << left << "\n"
             << sequence << "\n"
             << right << endl;

        CoreField f1(left);
        KumipuyoSeq seq(sequence);
        CoreField f2(right);

        cout << seq.toString() << endl;

        inputs.emplace_back(left, seq, right);
    }

    return inputs;
}

void learnFromPuyofu()
{
    FeatureParameter parameter(FLAGS_feature);
    Gazer gazer;

    gazer.initializeWith(1);

    while (true) {
        vector<FrameInput> inputs = readUntilGameEnd();
        if (inputs.empty())
            break;

        for (size_t i = 0; i + 1 < inputs.size(); ++i) {
            Evaluator evaluator(parameter);
            map<vector<Decision>, CollectedFeature> featureMap;
            CollectedFeature teacherFeature;
            bool teacherFound = false;

            cout << inputs[i].field.debugOutput() << endl
                 << inputs[i].seq.toString() << endl;

            Plan::iterateAvailablePlans(inputs[i].field, inputs[i].seq, 2, [&](const RefPlan& plan) {
                    CollectedFeature f = evaluator.evalWithCollectingFeature(plan, 1, gazer);
                    featureMap[plan.decisions()] = f;
                    if (plan.field() == inputs[i + 1].fieldAfter) {
                        teacherFound = true;
                        teacherFeature = f;
                    }
            });

            if (!teacherFound) {
                cout << "teacher didn't found" << endl;
                continue;
            }
            for (const auto& entry : featureMap) {
                updateFeature(&parameter, entry.second, teacherFeature);
            }
        }
    }

    cout << parameter.toString() << endl;

    CHECK(parameter.save("learned.txt"));
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    TsumoPossibility::initialize();

    if (FLAGS_interactive)
        learnWithInteractive();
    else
        learnFromPuyofu();

    return 0;
}
