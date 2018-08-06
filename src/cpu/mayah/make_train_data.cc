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

#include "cpu/mayah/evaluation_parameter.h"
#include "cpu/mayah/neural.h"
#include "cpu/mayah/mayah_ai.h"

DECLARE_string(feature);
DECLARE_int32(seed);

DEFINE_bool(show_field, false, "show field after each hand.");
DEFINE_int32(size, 100, "the number of case size.");
DEFINE_int32(offset, 0, "offset for random seed");

using namespace std;

static const Decision DECISIONS[] = {
    Decision(2, 3), Decision(3, 3), Decision(3, 1), Decision(4, 1),
    Decision(5, 1), Decision(1, 2), Decision(2, 2), Decision(3, 2),
    Decision(4, 2), Decision(5, 2), Decision(6, 2),
    Decision(1, 1), Decision(2, 1), Decision(4, 3), Decision(5, 3),
    Decision(6, 3), Decision(1, 0), Decision(2, 0), Decision(3, 0),
    Decision(4, 0), Decision(5, 0), Decision(6, 0),
};

struct Result {
    KumipuyoSeq seq;
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

void output_transition_to_file(const Result& result, std::ofstream& out) {
    CoreField cf;

    for (size_t i = 0; i < result.result.decisions.size(); ++i) {
        const Decision& d = result.result.decisions[i];
        PlainField pf = cf.toPlainField();
        out << pf.toString('.')
            << ','
            << result.seq.axis(i)
            << result.seq.child(i)
            << result.seq.axis(i + 1)
            << result.seq.child(i + 1)
            << ','
            << (50 - i)
            << ','
            << d.x
            << ','
            << d.r
            << ','
            << result.result.maxRensa
            << endl;

        cf.dropKumipuyo(d, result.seq.get(i));
        cf.simulateFast();
    }
}

int run_with_neural(int current_turn, CoreField current_field, KumipuyoSeq current_seq) {
    int max_chains = 0;

    for (int turn = current_turn; turn < 50; ++turn) {
        std::vector<NeuralNetRequest> requests;
        requests.push_back(NeuralNetRequest(current_field.toPlainField(), current_seq.get(0), current_seq.get(1), 50 - turn));
        std::vector<NeuralNetResponse> response = ask_puyo_server(requests);
        CHECK(!response.empty());

        double max_possibility = 0;
        int max_x = 0;
        int max_r = 0;
        for (int x = 1; x <= 6; ++x) {
            for (int r = 0; r < 4; ++r) {
                if (response[0].possibility[x-1][r] > max_possibility) {
                    max_possibility = response[0].possibility[x-1][r];
                    max_x = x;
                    max_r = r;
                }
            }
        }

        current_field.dropKumipuyo(Decision(max_x, max_r), current_seq.get(0));
        current_seq.dropFront();
        int chains = current_field.simulateFast();
        if (chains > max_chains) {
            max_chains = chains;
        }
    }

    return max_chains;
}

int run_with_existing(DebuggableMayahAI* ai, int current_turn, CoreField current_field, KumipuyoSeq current_seq) {
    int max_chains = 0;

    for (int turn = current_turn; turn < 50; ++turn) {
        ThoughtResult ai_thought_result = ai->thinkPlan(turn + 1, current_field, current_seq.subsequence(0, 2),
                                                        ai->myPlayerState(), ai->enemyPlayerState(),
                                                        PatternThinker::DEFAULT_DEPTH, PatternThinker::DEFAULT_NUM_ITERATION);

        const Plan& ai_plan = ai_thought_result.plan;
        if (ai_plan.decisions().empty()) {
            break;
        }

        current_field.dropKumipuyo(ai_plan.firstDecision(), current_seq.get(0));
        int chains = current_field.simulateFast();
        if (chains > max_chains) {
            max_chains = chains;
        }
        current_seq.dropFront();
    }

    return max_chains;
}

void run(DebuggableMayahAI* ai, KumipuyoSeq kumipuyo_seq) {
    CoreField current_field;

    for (int turn = 0; turn < 50; ++turn) {
        Kumipuyo current_kumipuyo = kumipuyo_seq.front();
        kumipuyo_seq.dropFront();

        const size_t decision_size = current_kumipuyo.isRep() ? 11 : 22;
        int max_chains = -1;
        Decision max_decision;

        for (size_t i = 0; i < decision_size; ++i) {
            const Decision& decision = DECISIONS[i];

            CoreField cf(current_field);
            cf.dropKumipuyo(decision, current_kumipuyo);
            int chains = cf.simulateFast();
            if (chains >= 5) {
                // considers as main chain.
                if (chains >= max_chains) {
                    max_chains = chains;
                    max_decision = decision;
                }
                continue;
            }

            // otherwise,
            int result_neural = run_with_neural(turn, cf, kumipuyo_seq);
            int result_existing = run_with_existing(ai, turn, cf, kumipuyo_seq);
            int result_chains = std::max(result_neural, result_existing);
            if (result_chains > max_chains) {
                max_chains = result_chains;
                max_decision = decision;
            }
        }

        current_field.dropKumipuyo(max_decision, current_kumipuyo);
        current_field.simulateFast();

        cout << "turn=" << turn << " max_chains=" << max_chains;
    }
}

int main(int argc, char* argv[]) {
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

    std::unique_ptr<DebuggableMayahAI> ai(new DebuggableMayahAI);
    ai->setUsesRensaHandTree(false);
    ai->setEvaluationParameterMap(paramMap);

    for (int i = 0; i < FLAGS_size; ++i) {
        KumipuyoSeq seq = KumipuyoSeqGenerator::generateACPuyo2Sequence();
        run(ai.get(), seq);
    }

    executor->stop();
    return 0;
}
