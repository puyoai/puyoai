#include <iostream>
#include <unordered_set>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/executor.h"
#include "base/time.h"
#include "base/wait_group.h"
#include "core/plan/plan.h"
#include "core/rensa/rensa_detector.h"
#include "core/client/ai/ai.h"
#include "core/core_field.h"
#include "core/decision.h"
#include "core/field_constant.h"
#include "core/field_pretty_printer.h"
#include "core/kumipuyo_seq_generator.h"
#include "core/pattern/decision_book.h"
#include "core/pattern/pattern_book.h"
#include "core/probability/column_puyo_list_probability.h"
#include "core/probability/puyo_set_probability.h"
#include "solver/endless.h"
#include "solver/puyop.h"

#include "evaluator.h"
#include "pattern_rensa_detector.h"
#include "rensa_evaluator.h"
#include "score_collector.h"
#include "shape_evaluator.h"
#include "mayah_ai.h"
#include "mixed_ai.h"
#include "yukina_ai.h"

using namespace std;

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
#if !defined(_MSC_VER)
    google::InstallFailureSignalHandler();
#endif

    (void)PuyoSetProbability::instanceSlow();
    (void)ColumnPuyoListProbability::instanceSlow();

    unique_ptr<YukinaAI> ai(new YukinaAI(argc, argv));

    Endless endless(std::move(ai));
    endless.setVerbose(true);
    //endless.setVerbose(FLAGS_show_field);

    // KumipuyoSeq seq = KumipuyoSeqGenerator::generateACPuyo2Sequence();
    // EndlessResult result = endless.run(seq);
    //
    // cout << seq.toString() << endl;
    // cout << makePuyopURL(seq, result.decisions) << endl;
    // cout << "score = " << result.score << " rensa = " << result.maxRensa;
    // if (result.zenkeshi)
    //     cout << " / ZENKESHI";
    // cout << endl;

    int zenkeshi = 0;
    vector<int> scores;
    vector<int> rensas;

    int main_rensa_count = 0;
    int main_rensa_score_sum = 0;

    const int N = 100;
    for (int i = 0; i < N; ++i) {
        KumipuyoSeq seq = KumipuyoSeqGenerator::generateACPuyo2Sequence();
        EndlessResult result = endless.run(seq);

        cout << seq.toString() << endl;
        cout << makePuyopURL(seq, result.decisions) << endl;
        cout << "score = " << result.score << " rensa = " << result.maxRensa;
        if (result.zenkeshi)
            cout << " / ZENKESHI";
        cout << endl;

        if (result.zenkeshi && result.hand < 8) {
            ++zenkeshi;
        }

        if (result.score > 10000) {
            main_rensa_count++;
            main_rensa_score_sum += result.score;
        }

        rensas.push_back(result.maxRensa);
        scores.push_back(result.score);
    }

    if (N > 1) {
        int num8 = 0;
        int num9 = 0;
        int num10 = 0;
        std::sort(scores.begin(), scores.end());
        int sum = 0;
        for (auto x : scores) {
            sum += x;
            if (x >= 80000)
                ++num8;
            if (x >= 90000)
                ++num9;
            if (x >= 100000)
                ++num10;
        }
        int average = sum / scores.size();

        cout << "        N = " << N << endl;
        cout << "      min = " << *min_element(scores.begin(), scores.end()) << endl;
        cout << "      max = " << *max_element(scores.begin(), scores.end()) << endl;
        cout << "  average = " << average << endl;
        cout << " zenkeshi = " << zenkeshi << endl;

        for (int i = 10; i <= 90; i += 10)
            cout << "      " << i << "% = " << scores[i] << endl;
        cout << " over 80K = " << num8 << endl;
        cout << " over 90K = " << num9 << endl;
        cout << "over 100K = " << num10 << endl;

        int rensa14 = 0;
        int rensa15 = 0;
        int rensa16 = 0;
        for (int r : rensas) {
            if (r >= 14)
                rensa14++;
            if (r >= 15)
                rensa15++;
            if (r >= 16)
                rensa16++;
        }
        cout << " rensa 14 = " << rensa14 << endl;
        cout << " rensa 15 = " << rensa15 << endl;
        cout << " rensa 16 = " << rensa16 << endl;

        cout << endl;
        cout << " main rensa count = " << main_rensa_count << endl;
        cout << " main rensa ave   = " << (main_rensa_score_sum / main_rensa_count) << endl;

    }

    return 0;
}
