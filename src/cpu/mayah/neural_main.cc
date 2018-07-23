#include <map>
#include <iostream>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/kumipuyo_seq.h"
#include "core/kumipuyo_seq_generator.h"
#include "core/plain_field.h"
#include "core/core_field.h"
#include "core/field_pretty_printer.h"
#include "cpu/mayah/neural.h"
#include "core/plan/plan.h"

// TODO(mayah): Implement with GUI!
int main(int argc, char* argv[]) {
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
#if !defined(_MSC_VER)
    google::InstallFailureSignalHandler();
#endif

    KumipuyoSeq current_seq = KumipuyoSeqGenerator::generateACPuyo2Sequence();
    PlainField current_field;

    for (int i = 0; i < 50; ++i) {
        // Show current field.
        FieldPrettyPrinter::print(current_field, current_seq);

        std::vector<NeuralNetRequest> requests;
        std::vector<Decision> decisions;
        std::map<Decision, std::vector<double>> qvalue;
        requests.push_back(NeuralNetRequest(current_field, current_seq.get(0), current_seq.get(1), 50 - i));
        decisions.push_back(Decision(0, 0));
        
        // For each
        Plan::iterateAvailablePlans(CoreField(current_field), current_seq, 1, [&](const RefPlan& plan){
            if (plan.isRensaPlan() && plan.score() >= 10000) {
                qvalue[plan.firstDecision()].push_back(plan.chains());
                return;
            }

            for (const auto axis : NORMAL_PUYO_COLORS) {
                for (const auto child : NORMAL_PUYO_COLORS) {
                    Kumipuyo kumipuyo(axis, child);
                    requests.push_back(NeuralNetRequest(plan.field().toPlainField(), current_seq.get(1), kumipuyo, 49 - i));
                    decisions.push_back(plan.firstDecision());
                }
            }
        });

        // ask neural net server.
        std::vector<NeuralNetResponse> resp = ask_puyo_server(requests);
        if (resp.empty()) {
            std::cout << "failed to ask puyo server" << std::endl;
            return 1;
        }

        printf("q = %f\n", resp[0].q);
        for (size_t i = 1; i < resp.size(); ++i) {
            // printf("x=%d r=%d q=%f\n", decisions[i].x, decisions[i].r, resp[i].q);
            qvalue[decisions[i]].push_back(resp[i].q);
        }

        // make decision -> q

        for (int x = 1; x <= 6; ++x) {
            for (int r = 0; r < 4; ++r) {
                double q_sum = 0;
                for (const auto& q : qvalue[Decision(x, r)]) {
                    q_sum += q;
                }
                if (!qvalue[Decision(x, r)].empty()) {
                    q_sum /= qvalue[Decision(x, r)].size();
                }
                printf("(%d, %d) = %6.4f/%6.3f  ", x, r, resp[0].possibility[x-1][r], q_sum);
            }
            printf("\n");
        }

        double best = resp[0].possibility[0][0];
        int best_x = 1;
        int best_r = 0;
        for (int x = 1; x <= 6; ++x) {
            for (int r = 0; r < 4; ++r) {
                if ((x == 1 && r == 3) || (x == 6 && r == 1)) {
                    continue;
                }

                if (resp[0].possibility[x-1][r] > best) {
                    best = resp[0].possibility[x-1][r];
                    best_x = x;
                    best_r = r;
                }
            }
        }

        std::string str;
        std::cout << "command? ";
        getline(std::cin, str);

        CoreField cf(current_field);
        cf.dropKumipuyo(Decision(best_x, best_r), current_seq.front());
        RensaResult rensa_result = cf.simulate();
        if (rensa_result.chains > 0) {
            std::cout << rensa_result << std::endl;
        }

        current_field = cf.toPlainField();
        current_seq.dropFront();     
    }

    return 0;
}