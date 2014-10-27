#include "mayah_ai.h"

#include <iomanip>
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
DECLARE_string(seq);
DECLARE_int32(seed);

DEFINE_bool(tweak, true, "true for tweaking");
DEFINE_bool(show_field, false, "show field after each hand");

using namespace std;

struct Result {
    int score;
    string msg;
};

struct RunResult {
    int sumScore;
    int mainRensaCount;
    int aveMainRensaScore;
    int over60000Count;
    int over70000Count;
    int over80000Count;
};

void removeNontokopuyoParameter(FeatureParameter* parameter)
{
    parameter->setValue(STRATEGY_ZENKESHI, 0);
    parameter->setValue(STRATEGY_INITIAL_ZENKESHI, 0);
    parameter->setValue(STRATEGY_TSUBUSHI, 0);
    parameter->setValue(STRATEGY_IBARA, 0);
    parameter->setValue(STRATEGY_SAISOKU, 0);
}

void runOnce(const FeatureParameter& parameter)
{
    auto ai = new DebuggableMayahAI;
    ai->setFeatureParameter(parameter);

    Endless endless(std::move(std::unique_ptr<AI>(ai)));
    endless.setVerbose(FLAGS_show_field);

    KumipuyoSeq seq = generateSequence();
    int score = endless.run(seq);

    cout << seq.toString() << endl;
    cout << "score = " << score << endl;
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
    int sumMainRensaScore = 0;
    int mainRensaCount = 0;
    int over60000Count = 0;
    int over70000Count = 0;
    int over80000Count = 0;
    for (int i = 0; i < N; ++i) {
        Result r = ps[i].get_future().get();
        sumScore += r.score;
        if (r.score >= 10000) {
            mainRensaCount++;
            sumMainRensaScore += r.score;
        }

        if (r.score >= 60000) {
            over60000Count++;
        }
        if (r.score >= 70000) {
            over70000Count++;
        }
        if (r.score >= 80000) {
            over80000Count++;
        }
        cout << r.msg;
    }

    int aveMainRensaScore = mainRensaCount > 0 ? sumMainRensaScore / mainRensaCount : 0;
    cout << "sum score  = " << sumScore << endl;
    cout << "ave score  = " << (sumScore / 100) << endl;
    cout << "main rensa = " << mainRensaCount << endl;
    cout << "ave main rensa = " << aveMainRensaScore << endl;
    cout << "over 60000 = " << over60000Count << endl;
    cout << "over 70000 = " << over70000Count << endl;
    cout << "over 80000 = " << over80000Count << endl;

    return RunResult { sumScore, mainRensaCount, aveMainRensaScore, over60000Count, over70000Count, over80000Count };
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

    if (!FLAGS_seq.empty() || FLAGS_seed >= 0) {
        runOnce(parameter);
    } else if (FLAGS_tweak) {
        map<int, RunResult> scoreMap;
        for (int x = 400; x <= 1000; x += 50) {
          cout << "current x = " << x << endl;
          parameter.setValue(MAX_RENSA_FIRE_PUYOS_LATE, 2, -x);
          scoreMap[x] = run(executor.get(), parameter);
        }
        for (const auto& m : scoreMap) {
            cout << setw(5) << m.first << " -> " << m.second.sumScore
                 << " / " << m.second.mainRensaCount
                 << " / " << m.second.aveMainRensaScore
                 << " / " << m.second.over60000Count
                 << " / " << m.second.over70000Count
                 << " / " << m.second.over80000Count
                 << endl;
        }
    } else {
        run(executor.get(), parameter);
    }

    executor->stop();
    return 0;
}
