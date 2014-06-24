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
    cout << "learning: D = " << D << endl;

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
            double curScore = parameter->score(key, curV);
            double teaScore = parameter->score(key, teaV);
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
    FeatureParameter parameter("feature.txt");
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

        cout << "our decision : "
             << currentDecision.first.x << ' '
             << currentDecision.first.r << ' '
             << currentDecision.second.x << ' '
             << currentDecision.second.r << ' '
             << "score=" << currentScore << endl;

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

#if 0
void learnFromPuyofu()
{
    FeatureParameter parameter("feature.txt");

    FrameInput frameInputs[3];
    bool shouldSkip = false;
    int fieldNum = 0;
    string left, sequence, right;

    MyPlayerInfo myPlayerInfo;
    EnemyInfo enemyInfo;

    while (cin >> left >> sequence >> right) {
        if (left == "===") {
            fieldNum = 0;
            shouldSkip = false;
            cout << "end" << endl;
            continue;
        }

        if (shouldSkip)
            continue;

        convert(left);
        convert(sequence);
        convert(right);

        vector<KumiPuyo> kumiPuyos;
        setKumiPuyo(sequence, kumiPuyos);
        kumiPuyos.resize(2);
        frameInputs[fieldNum % 3] = FrameInput(Field(left), kumiPuyos, Field(right));

        if (fieldNum >= 2) {
            const Field& current = frameInputs[fieldNum % 3].myField;
            const Field& previous = frameInputs[(fieldNum + 2) % 3].myField;
            // When we have an OJAMA puyo, skip this battle.
            if (current.countColorPuyos() != current.countPuyos())
                shouldSkip = true;
            // When puyo is vanished, we also skip this battle.
            // TODO(mayah): we have to consider this later.
            if (current.countColorPuyos() != previous.countColorPuyos() + 2)
                shouldSkip = true;

            if (!shouldSkip) {
                const Field& previous2 = frameInputs[(fieldNum + 1) % 3].myField;
                const vector<KumiPuyo>& kumiPuyos = frameInputs[(fieldNum + 1) % 3].kumiPuyos;

                myPlayerInfo.forceEstimatedField(previous2);
                enemyInfo.initializeWith(1);
                enemyInfo.updatePossibleRensas(frameInputs[(fieldNum + 1) % 3].enemyField, vector<KumiPuyo>());

                learner.learn(params, enemyInfo, previous2, kumiPuyos, current);
            }
        }

        ++fieldNum;
        cout << params.toString() << endl;
    }

    cout << params.toString() << endl;

    cout << "# learn = " << learner.numLearn() << endl;
    cout << "# match = " << learner.numMatchedTeacher() << endl;
    cout << "# ratio = " << learner.ratioMatchedTeacher() << endl;

    if (params.save("feature_learned.txt")) {
        cout << "Saved as feature_learned.txt" << endl;
    } else {
        cout << "Save faild..." << endl;
    }
}
#endif

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    TsumoPossibility::initialize();

    learnWithInteractive();

    return 0;
}
