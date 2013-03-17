#include <algorithm>
#include <iostream>
#include <math.h>
#include <set>
#include <stdio.h>
#include <string>

#include <boost/thread.hpp>
#include <boost/thread/future.hpp>
#include <glog/logging.h>

#include "ai.h"
#include "enemy_info.h"
#include "evaluation_feature.h"
#include "evaluation_feature_collector.h"
#include "field.h"
#include "plan.h"
#include "player_info.h"
#include "puyo.h"
#include "puyo_possibility.h"

using namespace std;

const double LEARNING_COEF = 1 / 1000.0;

struct FrameInput {
    FrameInput() {}
    FrameInput(const Field& left, const vector<KumiPuyo>& kumiPuyos, const Field& right) :
        myField(left), kumiPuyos(kumiPuyos), enemyField(right) {}

    Field myField;
    vector<KumiPuyo> kumiPuyos;
    Field enemyField;
};

void convert(string& str)
{
    for (string::size_type i = 0; i < str.size(); ++i) {
        switch (str[i]) {
        case '0': str[i] = '0' + EMPTY; break;
        case '1': str[i] = '0' + RED; break;
        case '2': str[i] = '0' + BLUE; break;
        case '3': str[i] = '0' + YELLOW; break;
        case '4': str[i] = '0' + GREEN; break;
        case '5': str[i] = '0' + OJAMA; break;
        default: DCHECK(false);
        }
    }
}

void updatePlanParam(EvaluationParams& params, const PlanEvaluationFeature& currentFeature, const PlanEvaluationFeature& teacherFeature)
{
    for (int i = 0; i < SIZE_OF_PLAN_FEATURE_PARAM; ++i) {
        PlanFeatureParam paramName = toPlanFeatureParam(i);
        double dT = (currentFeature.get(paramName) - teacherFeature.get(paramName)) * LEARNING_COEF;
        cout << "learning: " << paramName << ' '
             << params.get(paramName) << ' '
             << currentFeature.get(paramName) << ' ' 
             << teacherFeature.get(paramName) << ' '
             << dT << endl;
        params.add(paramName, -dT);
    }

    for (int i = 0; i < SIZE_OF_PLAN_RANGE_FEATURE_PARAM; ++i) {
        PlanRangeFeatureParam paramName = toPlanRangeFeatureParam(i);
        double dT = (currentFeature.get(paramName) - teacherFeature.get(paramName)) * LEARNING_COEF;
        cout << "learning: " << paramName << ' '
             << params.get(paramName, currentFeature.get(paramName)) << ' '
             << params.get(paramName, teacherFeature.get(paramName)) << ' '
             << currentFeature.get(paramName) << ' ' 
             << teacherFeature.get(paramName) << ' '
             << dT << endl;
        params.add(paramName, currentFeature.get(paramName), -dT);
        params.add(paramName, teacherFeature.get(paramName), dT);
    }

    {
        map<pair<PlanSparseFeatureParam, int>, int> diff;
        for (auto it = currentFeature.sparseFeatures().begin(); it != currentFeature.sparseFeatures().end(); ++it)
            diff[*it] += 1;
        for (auto it = teacherFeature.sparseFeatures().begin(); it != teacherFeature.sparseFeatures().end(); ++it)
            diff[*it] -= 1;

        cout << "*** " << diff.size() << endl;

        for (auto it = diff.begin(); it != diff.end(); ++it) {
            double dT = it->second * LEARNING_COEF;
            params.add(it->first.first, it->first.second, -dT);
        }
    }    
}

void updateRensaParam(EvaluationParams& params, const RensaEvaluationFeature& currentFeature, const RensaEvaluationFeature& teacherFeature)
{
    for (int i = 0; i < SIZE_OF_RENSA_FEATURE_PARAM; ++i) {
        RensaFeatureParam paramName = toRensaFeatureParam(i);
        double dT = (currentFeature.get(paramName) - teacherFeature.get(paramName)) * LEARNING_COEF;
        cout << "learning: " << paramName << ' '
             << params.get(paramName) << ' '
             << currentFeature.get(paramName) << ' ' 
             << teacherFeature.get(paramName) << ' '
             << dT << endl;
        params.add(paramName, -dT);
    }

    for (int i = 0; i < SIZE_OF_RENSA_RANGE_FEATURE_PARAM; ++i) {
        RensaRangeFeatureParam paramName = toRensaRangeFeatureParam(i);
        double dT = (currentFeature.get(paramName) - teacherFeature.get(paramName)) * LEARNING_COEF;
        cout << "learning: " << paramName << ' '
             << params.get(paramName, currentFeature.get(paramName)) << ' '
             << params.get(paramName, teacherFeature.get(paramName)) << ' '
             << currentFeature.get(paramName) << ' ' 
             << teacherFeature.get(paramName) << ' '
             << dT << endl;
        params.add(paramName, currentFeature.get(paramName), -dT);
        params.add(paramName, teacherFeature.get(paramName), dT);
    }
}

void updateParam(EvaluationParams& params, const EvaluationFeature& currentFeature, const EvaluationFeature& teacherFeature)
{
    updateRensaParam(params, currentFeature.findBestRensaFeature(params), teacherFeature.findBestRensaFeature(params));
    updatePlanParam(params, currentFeature.planFeature(), teacherFeature.planFeature());
}

class Learner {
public:
    Learner() : m_numLearn(0), m_numMatchedTeacher(0) {}

    void learn(EvaluationParams& params, const EnemyInfo& enemyInfo,
               const Field& currentField, const vector<KumiPuyo>& kumiPuyos,
               const Field& teacherField);

    int numLearn() const { return m_numLearn; }
    int numMatchedTeacher() const { return m_numMatchedTeacher; }
    double ratioMatchedTeacher() const { return 100 * numMatchedTeacher() / numLearn(); }
private:
    int m_numLearn;
    int m_numMatchedTeacher;
};

void Learner::learn(EvaluationParams& params, const EnemyInfo& enemyInfo,
                    const Field& currentField, const vector<KumiPuyo>& kumiPuyos,
                    const Field& teacherField)
{
    ++m_numLearn;
    cout << "learn" << endl;
    cout << currentField.getDebugOutput() << endl;
    for (const KumiPuyo& kumiPuyo : kumiPuyos) {
        cout << kumiPuyo.toString();
    }
    cout << endl;

    vector<Plan> plans;
    findAvailablePlans(currentField, 2, kumiPuyos, plans);
    if (plans.empty())
        return;

    // --- スコアを付ける
    vector<EvaluationFeature> features(plans.size());
    // TODO(mayah): We would like to use future instead of shared_future, however mac requires libc++ to do so.
    // If we use libc++, we have to recompile gtest as well...
    vector<boost::shared_future<void>> futures;
    //futures.reserve(plans.size());
    for (size_t i = 0; i < plans.size(); ++i) {
        futures.push_back(boost::async(boost::launch::async, [&features, &plans, &enemyInfo, i]() {
            EvaluationFeatureCollector::collectFeatures(features[i], plans[i], AI::NUM_KEY_PUYOS, 0, enemyInfo);
        }));
    }

    vector<pair<double, size_t> > scores(plans.size());
    for (size_t i = 0; i < plans.size(); ++i) {
        if (plans[i].isRensaPlan()) {
            scores[i] = make_pair(-1000000, i);
        } else {
            futures[i].wait();
            double score = features[i].calculateScore(params);
            scores[i] = make_pair(score, i);
        }        
    }

    sort(scores.begin(), scores.end(), greater<pair<double, size_t> >());

    // --- 教師の盤面と、そのスコアを求める。
    size_t teacherIndex = scores.size();
    for (size_t i = 0; i < scores.size(); ++i) {
        if (plans[scores[i].second].isRensaPlan())
            continue;

        if (plans[scores[i].second].field() == teacherField) {
            teacherIndex = i;
            break;
        }
    }
    if (teacherIndex == scores.size()) {
        cerr << "We couldn't find teacher field. It might be a bug?" << endl;
        return;
    }

    // --- 教師の盤面と最もスコアが高い盤面が一緒であれば、何も更新する必要はない。
    if (teacherIndex == 0) {
        ++m_numMatchedTeacher;
        return;
    }

    // --- そうでなければ、スコア付けに問題があったということで、スコアを更新する。
    const EvaluationFeature& currentFeature = features[scores[0].second];
    const EvaluationFeature& teacherFeature = features[scores[teacherIndex].second];

    cout << "teacher decision = " << plans[scores[teacherIndex].second].decisionText() << " : " << scores[teacherIndex].first << endl;
    cout << teacherFeature.toString() << endl;
    cout << "our decision = " << plans[scores[0].second].decisionText() << " : " << scores[0].first << endl;
    cout << currentFeature.toString() << endl;

    updateParam(params, currentFeature, teacherFeature);
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    TsumoPossibility::initialize();

    // TODO(mayah): Maybe we have to use constant variable name here.
    EvaluationParams params("feature.txt");

    FrameInput frameInputs[3];
    bool shouldSkip = false;
    int fieldNum = 0;
    string left, sequence, right;

    MyPlayerInfo myPlayerInfo;
    EnemyInfo enemyInfo;

    Learner learner;

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
    return 0;
}
