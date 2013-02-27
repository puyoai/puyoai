#include <glog/logging.h>
#include <iostream>
#include <math.h>
#include <set>
#include <stdio.h>
#include <string>

#include "evaluation_feature.h"
#include "evaluation_feature_collector.h"
#include "field.h"
#include "plan.h"
#include "player_info.h"
#include "puyo.h"
#include "puyo_possibility.h"

using namespace std;

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

double dsigmoid(double x)
{
    const double delta = 256.0 / 7.0;

    if (x <= -256 || 256.0 <= x)
        return 0.0F;

    double dn = exp(-x / delta);
    double dd = delta * (dn + 1.0) * (dn * 1.0);
    return dn / dd;
}

void learn(EvaluationParams& params, 
           const MyPlayerInfo& myPlayerInfo, 
           const Field& currentField, const vector<KumiPuyo>& kumiPuyos,
           const Field& teacherField)
{
    cout << "learn" << endl;
    cout << currentField.getDebugOutput() << endl;

    vector<Plan> plans;
    findAvailablePlans(currentField, 2, kumiPuyos, plans);
    if (plans.empty())
        return;

    // --- スコアを付ける
    vector<pair<float, size_t> > scores(plans.size());
    vector<EvaluationFeature> features(plans.size());
    for (size_t i = 0; i < plans.size(); ++i) {
        EvaluationFeatureCollector::collectFeatures(features[i], plans[i], myPlayerInfo);

        if (plans[i].isRensaPlan()) {
            scores[i] = make_pair(-1000000, i);
        } else {
            double score = params.calculateScore(features[i]);
            scores[i] = make_pair(score, i);
        }
    }

    sort(scores.begin(), scores.end(), greater<pair<float, size_t> >());

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
    if (teacherIndex == 0)
        return;

    // --- そうでなければ、スコア付けに問題があったということで、スコアを更新する。
    // 教師のスコアより高い得点が与えられている特徴値に対して、
    // - 教師の特徴値が大きければ、係数を増やす
    // - 
    // ただし、定数倍の別解を除去する必要があるので、２連鎖の得点を固定する。
    const EvaluationFeature& teacherFeature = features[scores[teacherIndex].second];
    const EvaluationFeature& currentFeature = features[scores[0].second];

    for (int i = 0; i < EvaluationFeature::SIZE_OF_FEATURE_PARAM; ++i) {
        EvaluationFeature::FeatureParam name = EvaluationFeature::param(i);
        double dT = (currentFeature.get(name) - teacherFeature.get(name));
        cout << "learning: " << name << ' '
             << params.get(name) << ' '
             << currentFeature.get(name) << ' ' 
             << teacherFeature.get(name) << ' '
             << dT << endl;
        params.add(name, -dT / 100.0);
    }

    for (int i = 0; i < EvaluationFeature::SIZE_OF_RANGE_FEATURE_PARAM; ++i) {
        EvaluationFeature::RangeFeatureParam name = EvaluationFeature::rangeParam(i);
        if (currentFeature.get(name) == teacherFeature.get(name))
            continue;
        double dT = (currentFeature.get(name) - teacherFeature.get(name)) / 1000.0;
        cout << "learning (2): " << name << ' '
             << params.get(name, currentFeature.get(name)) << ' '
             << params.get(name, teacherFeature.get(name)) << ' '
             << currentFeature.get(name) << ' ' 
             << teacherFeature.get(name) << ' '
             << dT << endl;
        params.add(name, currentFeature.get(name), -dT);
        params.add(name, teacherFeature.get(name), dT);
    }
}

int main(void)
{
    TsumoPossibility::initialize();
    
    EvaluationParams params;

    pair<Field, vector<KumiPuyo>> fieldAndKumiPuyos[3];
    bool shouldSkip = false;
    int fieldNum = 0;
    string left, sequence, right;
    MyPlayerInfo myPlayerInfo;

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

        vector<KumiPuyo> kumiPuyos;
        setKumiPuyo(sequence, kumiPuyos);
        kumiPuyos.resize(2);
        fieldAndKumiPuyos[fieldNum % 3] = make_pair(Field(left), kumiPuyos);

        if (fieldNum >= 2) {
            const Field& current = fieldAndKumiPuyos[fieldNum % 3].first;
            const Field& previous = fieldAndKumiPuyos[(fieldNum + 2) % 3].first;
            // When we have an OJAMA puyo, skip this battle.
            if (current.countColorPuyos() != current.countPuyos()) 
                shouldSkip = true;
            // When puyo is vanished, we also skip this battle.
            // TODO(mayah): we have to consider this later.
            if (current.countColorPuyos() != previous.countColorPuyos() + 2)
                shouldSkip = true;

            if (!shouldSkip) {
                const Field& previous2 = fieldAndKumiPuyos[(fieldNum + 1) % 3].first;
                const vector<KumiPuyo>& kumiPuyos = fieldAndKumiPuyos[(fieldNum + 1) % 3].second;

                myPlayerInfo.forceEstimatedField(previous2);
                myPlayerInfo.updateMainRensa(kumiPuyos, params);

                learn(params, myPlayerInfo, previous2, kumiPuyos, current);
            }
        }

        ++fieldNum;
        cout << params.toString() << endl;
    }

    cout << params.toString() << endl;
    if (params.save("feature.bin")) {
        cout << "Saved as feature.bin" << endl;
    } else {
        cout << "Save faild..." << endl;
    }
    return 0;
}
