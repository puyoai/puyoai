#include "mayah_ai.h"

#include <iostream>
#include <future>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/executor.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/client/ai/endless/endless.h"
#include "core/sequence_generator.h"

#include "feature_parameter.h"

using namespace std;

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    TsumoPossibility::initialize();

    Executor executor(4);
    executor.start();

    struct Result {
        int score;
        string msg;
    };

    int N = 100;
    vector<promise<Result>> ps(N);

    int sumScore = 0;
    for (int i = 0; i < N; ++i) {
        auto f = [i, &ps]() {
            auto ai = new DebuggableMayahAI;
            Endless endless(std::move(std::unique_ptr<AI>(ai)));
            stringstream ss;
            KumipuyoSeq seq = generateRandomSequenceWithSeed(i);
            int score = endless.run(seq);
            ss << "case " << i << ": "
               << "score = " << score << endl;

            ps[i].set_value(Result{score, ss.str()});
        };
        executor.submit(f);
    }

    cout << "all submitted." << endl;

    for (int i = 0; i < N; ++i) {
        Result r = ps[i].get_future().get();
        sumScore += r.score;
        cout << r.msg;
    }

    cout << "sum score = " << sumScore << endl;
    cout << "ave score = " << (sumScore / 100) << endl;

    return 0;
}
