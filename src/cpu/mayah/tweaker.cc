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

DEFINE_int32(core, 8, "the number of parallel tasks");
DECLARE_string(feature);

using namespace std;

struct Result {
    int score;
    string msg;
};

void removeNontokopuyoParameter(FeatureParameter* parameter)
{
    parameter->setValue(STRATEGY_ZENKESHI, 0);
    parameter->setValue(STRATEGY_TSUBUSHI, 0);
}

int run(Executor* executor, const FeatureParameter& parameter)
{
    const int N = 100;
    vector<promise<Result>> ps(N);

    int sumScore = 0;
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

    for (int i = 0; i < N; ++i) {
        Result r = ps[i].get_future().get();
        sumScore += r.score;
        cout << r.msg;
    }

    cout << "sum score = " << sumScore << endl;
    cout << "ave score = " << (sumScore / 100) << endl;

    return sumScore;
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    TsumoPossibility::initialize();

    Executor executor(FLAGS_core);
    executor.start();

    FeatureParameter parameter(FLAGS_feature);
    removeNontokopuyoParameter(&parameter);

#if 0
    map<int, int> scoreMap;
    for (int x = -15; x <= -5; x += 1) {
        cout << "current x = " << x << endl;
        parameter.setValue(FIELD_USHAPE, x);
        scoreMap[x] = run(&executor, parameter);
    }
    for (const auto& m : scoreMap) {
        cout << m.first << " -> " << m.second << endl;
    }

#else
    run(&executor, parameter);
#endif


    return 0;
}
