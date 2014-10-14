#include "mayah_ai.h"

#include <iostream>
#include <future>
#include <sstream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/executor.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/client/ai/endless/endless.h"
#include "core/sequence_generator.h"

#include "feature_parameter.h"

DECLARE_string(feature);

using namespace std;

struct Result {
    int score;
    string msg;
};

struct RunResult {
    int sumScore;
    int mainRensaCount;
};

void removeNontokopuyoParameter(FeatureParameter* parameter)
{
    parameter->setValue(STRATEGY_ZENKESHI, 0);
    parameter->setValue(STRATEGY_TSUBUSHI, 0);
}

RunResult run(Executor* executor, const FeatureParameter& parameter)
{
    const int N = 100;
    vector<promise<Result>> ps(N);

    for (int i = 0; i < N; ++i) {
        auto f = [i, &parameter, &ps]() {
            auto ai = new DebuggableMayahAI;
            ai->setFeatureParameter(parameter);
            Endless endless(std::move(std::unique_ptr<AI>(ai)));
            stringstream ss;
            KumipuyoSeq seq = generateRandomSequenceWithSeed(i);
            int score = endless.run(seq);
            ss << "case " << i << ": "
               << "score = " << score << endl;

            ps[i].set_value(Result{score, ss.str()});
        };
        executor->submit(f);
    }

    int sumScore = 0;
    int mainRensaCount = 0;
    for (int i = 0; i < N; ++i) {
        Result r = ps[i].get_future().get();
        sumScore += r.score;
        if (r.score >= 10000)
            mainRensaCount++;
        cout << r.msg;
    }

    cout << "sum score  = " << sumScore << endl;
    cout << "ave score  = " << (sumScore / 100) << endl;
    cout << "main rensa = " << mainRensaCount << endl;

    return RunResult { sumScore, mainRensaCount };
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    TsumoPossibility::initialize();

    unique_ptr<Executor> executor = Executor::makeDefaultExecutor();

    FeatureParameter parameter(FLAGS_feature);
    removeNontokopuyoParameter(&parameter);

#if 1
    map<int, RunResult> scoreMap;
    for (int x = 0; x <= 100; x += 10) {
        cout << "current x = " << x << endl;
        parameter.setValue(FIRE_POINT_TABOO, -x);
        scoreMap[x] = run(executor.get(), parameter);
    }
    for (const auto& m : scoreMap) {
        cout << m.first << " -> " << m.second.sumScore << " / " << m.second.mainRensaCount << endl;
    }

#else
    run(executor.get(), parameter);
#endif

    executor->stop();
    return 0;
}
