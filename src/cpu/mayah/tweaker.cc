#include "mayah_ai.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <future>
#include <sstream>
#include <random>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/executor.h"
#include "core/kumipuyo_seq_generator.h"
#include "core/probability/puyo_set_probability.h"
#include "solver/endless.h"
#include "solver/puyop.h"

#include "evaluation_parameter.h"

DECLARE_string(feature);
DECLARE_string(seq);
DECLARE_int32(seed);

DEFINE_bool(once, false, "true if running only once.");
#if 0
DEFINE_int32(auto_count, 0, "run auto tweaker for this count.");
#endif
DEFINE_bool(show_field, false, "show field after each hand.");
DEFINE_int32(size, 100, "the number of case size.");
DEFINE_int32(offset, 0, "offset for random seed");

using namespace std;

struct Result {
    EndlessResult result;
    string msg;
};

struct RunResult {
    int numZenkeshi;
    int sumScore;
    int mainRensaCount;
    int aveMainRensaScore;
    int over40000Count;
    int over60000Count;
    int over70000Count;
    int over80000Count;
    int over100000Count;

    int resultScore() {
        return mainRensaCount * 20 + over60000Count * 6 + over70000Count + over80000Count;
    }
};

#if 0
class ParameterTweaker {
public:
    ParameterTweaker() : mt_(random_device()())
    {
        for (const auto& ef : EvaluationFeature::all()) {
            if (ef.tweakable()) {
                tweakableFeatures_.push_back(ef);
            }
        }

        for (const auto& ef : EvaluationSparseFeature::all()) {
            if (ef.tweakable()) {
                tweakableSparseFeatures_.push_back(ef);
            }
        }
    }

    void tweakParameter(EvaluationParameterMap* paramMap)
    {
        if (tweakableFeatures_.empty() && tweakableSparseFeatures_.empty())
            return;

        EvaluationParameter* parameter = paramMap->mutableDefaultParameter();

        size_t N = tweakableFeatures_.size() + tweakableSparseFeatures_.size();

        // Currently consider only non sparse keys.
        std::uniform_int_distribution<> dist(0, N - 1);
        size_t i = dist(mt_);
        if (i < tweakableFeatures_.size()) {
            tweak(tweakableFeatures_[i], parameter);
        } else {
            tweak(tweakableSparseFeatures_[i - tweakableFeatures_.size()], parameter);
        }
    }

    void tweak(const EvaluationFeature& feature, EvaluationParameter* parameter)
    {
        double original = parameter->getValue(feature.key());
        normal_distribution<> dist(0, 10.0);
        double newValue = original + dist(mt_);

        cout << feature.str() << ": " << original << " -> " << newValue << endl;
        parameter->setValue(feature.key(), newValue);
    }

    void tweak(const EvaluationSparseFeature& feature, EvaluationParameter* parameter)
    {
        vector<double> values = parameter->getValues(feature.key());
        double d = normal_distribution<>(0, 10.0)(mt_);
        size_t k = uniform_int_distribution<>(0, values.size() - 1)(mt_);

        if ((feature.ascending() && d > 0) || (feature.descending() && d < 0)) {
            for (size_t i = 0; i < k; ++i) {
                values[i] += d;
            }
        } else {
            for (size_t i = k; i < values.size(); ++i) {
                values[i] += d;
            }
        }

        parameter->setValues(feature.key(), values);
    }

private:
    mt19937 mt_;
    vector<EvaluationFeature> tweakableFeatures_;
    vector<EvaluationSparseFeature> tweakableSparseFeatures_;
};
#endif

void runOnce(const EvaluationParameterMap& paramMap)
{
    auto ai = new DebuggableMayahAI;
    ai->setUsesRensaHandTree(false);
    ai->setEvaluationParameterMap(paramMap);

    std::unique_ptr<AI> ai_ptr(ai);
    Endless endless(std::move(ai_ptr));
    endless.setVerbose(FLAGS_show_field);

    KumipuyoSeq seq = KumipuyoSeqGenerator::generateACPuyo2Sequence();
    EndlessResult result = endless.run(seq);

    cout << seq.toString() << endl;
    cout << makePuyopURL(seq, result.decisions) << endl;
    cout << "score = " << result.score << " rensa = " << result.maxRensa;
    if (result.zenkeshi)
        cout << " / ZENKESHI";
    cout << endl;
}

RunResult run(Executor* executor, const EvaluationParameterMap& paramMap)
{
    const int N = FLAGS_size;
    vector<promise<Result>> ps(N);

    for (int i = 0; i < N; ++i) {
        auto f = [i, &paramMap, &ps]() {
            auto ai = new DebuggableMayahAI;
            ai->setUsesRensaHandTree(false);
            ai->setEvaluationParameterMap(paramMap);

            std::unique_ptr<AI> ai_ptr(ai);
            Endless endless(std::move(ai_ptr));

            stringstream ss;
            KumipuyoSeq seq = KumipuyoSeqGenerator::generateACPuyo2SequenceWithSeed(i + FLAGS_offset);
            EndlessResult result = endless.run(seq);
            ss << "case " << setw(2) << i << ": "
               << "score=" << setw(6) << result.score << " rensa=" << setw(2) << result.maxRensa;
            if (result.zenkeshi)
                ss << " / ZENKESHI";
            ss << endl;

            ps[i].set_value(Result{result, ss.str()});
        };
        executor->submit(f);
    }

    int numZenkeshi = 0;
    int sumScore = 0;
    int sumMainRensaScore = 0;
    int mainRensaCount = 0;
    int over40000Count = 0;
    int over60000Count = 0;
    int over70000Count = 0;
    int over80000Count = 0;
    int over100000Count = 0;
    int overRensa13Count = 0;
    int overRensa14Count = 0;
    int overRensa15Count = 0;

    vector<pair<int, int>> scores;
    for (int i = 0; i < N; ++i) {
        Result r = ps[i].get_future().get();
        cout << r.msg;
        if (r.result.zenkeshi && r.result.hand < 8) {
            numZenkeshi++;
            continue;
        }
        int score = r.result.score;
        sumScore += score;
        scores.push_back(make_pair(score, i + FLAGS_offset));
        if (score >= 10000) {
            mainRensaCount++;
            sumMainRensaScore += score;
        }
        if (score >= 40000) { over40000Count++; }
        if (score >= 60000) { over60000Count++; }
        if (score >= 70000) { over70000Count++; }
        if (score >= 80000) { over80000Count++; }
        if (score >= 100000) { over100000Count++; }
        if (r.result.maxRensa >= 13) { overRensa13Count++; }
        if (r.result.maxRensa >= 14) { overRensa14Count++; }
        if (r.result.maxRensa >= 15) { overRensa15Count++; }
    }

    sort(scores.begin(), scores.end());

    double mean = static_cast<double>(sumScore) / scores.size();
    double variance = 0.0;
    for (size_t i = 0; i < scores.size(); ++i) {
        variance += (scores[i].first - mean) * (scores[i].first - mean);
    }
    double deviation = scores.size() >= 2 ? sqrt(variance / (scores.size() - 1)) : 0;

    int aveMainRensaScore = mainRensaCount > 0 ? sumMainRensaScore / mainRensaCount : 0;
    cout << "zenkeshi   = " << numZenkeshi << endl;
    cout << "sum score  = " << sumScore << endl;
    cout << "ave score  = " << (sumScore / scores.size()) << endl;
    cout << "median     = " << scores[scores.size() / 2].first << endl;
    cout << "deviation  = " << deviation << endl;
    cout << "main rensa = " << mainRensaCount << endl;
    cout << "main rensa % = " << (100.0 * mainRensaCount / scores.size()) << endl;
    cout << "ave main rensa = " << aveMainRensaScore << endl;
    cout << "over  40000 = " << over40000Count << endl;
    cout << "over  60000 = " << over60000Count << endl;
    cout << "over  70000 = " << over70000Count << endl;
    cout << "over  80000 = " << over80000Count << endl;
    cout << "over 100000 = " << over100000Count << endl;
    cout << "over 13 chn = " << overRensa13Count << endl;
    cout << "over 14 chn = " << overRensa14Count << endl;
    cout << "over 15 chn = " << overRensa15Count << endl;
    if (scores.size() >= 10) {
        for (int i = 0; i < 5; ++i) {
            cout << "  seed " << scores[i].second << " -> " << scores[i].first << endl;
        }
    }

    return RunResult { numZenkeshi, sumScore, mainRensaCount, aveMainRensaScore,
            over40000Count, over60000Count, over70000Count, over80000Count, over100000Count };
}

#if 0
void runAutoTweaker(Executor* executor, const EvaluationParameterMap& original, int num)
{
    cout << "Run with the original parameter." << endl;
    EvaluationParameterMap currentBestParameter(original);
    RunResult currentBestResult = run(executor, original);

    cout << "original score = " << currentBestResult.resultScore() << endl;

    ParameterTweaker tweaker;

    for (int i = 0; i < num; ++i) {
        EvaluationParameterMap parameter(currentBestParameter);
        tweaker.tweakParameter(&parameter);

        RunResult result = run(executor, parameter);
        cout << "score = " << result.resultScore() << endl;

        if (currentBestResult.resultScore() < result.resultScore()) {
            currentBestResult = result;
            currentBestParameter = parameter;

            cout << "Best parameter is updated." << endl;
            cout << currentBestParameter.toString() << endl;

            CHECK(currentBestParameter.save("best-parameter.txt"));
        }
    }
}
#endif

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
#if !defined(_MSC_VER)
    google::InstallFailureSignalHandler();
#endif

    unique_ptr<Executor> executor = Executor::makeDefaultExecutor();

    EvaluationParameterMap paramMap;
    if (!paramMap.load(FLAGS_feature)) {
        std::string filename = string(SRC_DIR) + "/cpu/mayah/" + FLAGS_feature;
        if (!paramMap.load(filename))
            CHECK(false) << "parameter cannot be loaded correctly.";
    }
    paramMap.removeNontokopuyoParameter();

    if (!FLAGS_seq.empty() || FLAGS_seed >= 0) {
        runOnce(paramMap);
    } else if (FLAGS_once) {
        run(executor.get(), paramMap);
#if 0
    } else if (FLAGS_auto_count > 0) {
        runAutoTweaker(executor.get(), paramMap, FLAGS_auto_count);
#endif
    } else {
        typedef tuple<double, double> ScoreMapKey;

        map<ScoreMapKey, RunResult> scoreMap;
        for (double x = 0; x <= 10; x += 1) {
            for (double y = 0; y <= 10; y += 1) {
              cout << "current (x, y) = " << x << ' ' << y << ' ' << endl;

              paramMap.mutableMainRensaParamSet()->setParam(EvaluationMode::INITIAL, HIGHER_PUYO_THAN_IGNITION_LINEAR, -x);
              paramMap.mutableMainRensaParamSet()->setParam(EvaluationMode::EARLY, HIGHER_PUYO_THAN_IGNITION_LINEAR, -x);
              paramMap.mutableMainRensaParamSet()->setParam(EvaluationMode::MIDDLE, HIGHER_PUYO_THAN_IGNITION_LINEAR, -x);
              paramMap.mutableMainRensaParamSet()->setParam(EvaluationMode::LATE, HIGHER_PUYO_THAN_IGNITION_LINEAR, -x);

              paramMap.mutableMainRensaParamSet()->setParam(EvaluationMode::INITIAL, HIGHER_PUYO_THAN_IGNITION_SQUARE, -y);
              paramMap.mutableMainRensaParamSet()->setParam(EvaluationMode::EARLY, HIGHER_PUYO_THAN_IGNITION_SQUARE, -y);
              paramMap.mutableMainRensaParamSet()->setParam(EvaluationMode::MIDDLE, HIGHER_PUYO_THAN_IGNITION_SQUARE, -y);
              paramMap.mutableMainRensaParamSet()->setParam(EvaluationMode::LATE, HIGHER_PUYO_THAN_IGNITION_SQUARE, -y);

              scoreMap[ScoreMapKey(x, y)] = run(executor.get(), paramMap);
            }
        }

        for (const auto& m : scoreMap) {
            cout << setw(5) << get<0>(m.first) << ' ' << get<1>(m.first)
                 << " -> " << m.second.sumScore
                 << " / " << m.second.mainRensaCount
                 << " / " << m.second.aveMainRensaScore
                 << " / " << m.second.over40000Count
                 << " / " << m.second.over60000Count
                 << " / " << m.second.over70000Count
                 << " / " << m.second.over80000Count
                 << " / " << m.second.over100000Count
                 << endl;
        }
    }

    executor->stop();
    return 0;
}
