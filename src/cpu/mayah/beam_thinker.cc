#include "beam_thinker.h"

#include <map>
#include <set>
#include <unordered_set>

#include "base/time.h"
#include "base/wait_group.h"
#include "core/field_pretty_printer.h"
#include "core/kumipuyo_seq_generator.h"
#include "core/plan/plan.h"
#include "core/rensa/rensa_detector.h"

#include <iostream>

DEFINE_int32(beam_width, 400, "beam width");
DEFINE_int32(beam_depth, 50, "beam depth");
DEFINE_int32(beam_num, 12, "beam iteration number");

using namespace std;

namespace {

struct SearchResult {
    std::set<Decision> firstDecisions;
    int maxChains = 0;
};

struct State {
    State(const CoreField& field, const Decision& firstDecision, double stateScore, int maxChains,
          int total_frames) :
        field(field),
        firstDecision(firstDecision),
        stateScore(stateScore),
        maxChains(maxChains),
        total_frames(total_frames)
    {
    }

#if 0
    friend bool operator<(const State& lhs, const State& rhs) { return lhs.stateScore < rhs.stateScore; }
#endif
    friend bool operator>(const State& lhs, const State& rhs) { return lhs.stateScore > rhs.stateScore; }

    CoreField field;
    Decision firstDecision;
    double stateScore = 0;
    int maxChains = 0;

    int total_frames = 0;
    int pending_enemy_score = 0;
    int pending_enemy_ojama_drop_frame = 0;
};

std::pair<double, int> evalSuperLight(const CoreField& fieldBeforeRensa)
{
    int maxChains = 0;
    auto callback = [&maxChains](CoreField&& complementedField, const ColumnPuyoList& /*cpl*/) {
        maxChains = std::max(maxChains, complementedField.simulateFast());
    };
    static const bool prohibits[FieldConstant::MAP_WIDTH] {};
    RensaDetector::detectByDropStrategy(fieldBeforeRensa, prohibits, PurposeForFindingRensa::FOR_FIRE, 2, 13, callback);

    double maxScore = 0;
    maxScore += maxChains * 1000;

    double averageHeight = 0;
    for (int x = 1; x <= 6; ++x)
        averageHeight += fieldBeforeRensa.height(x);
    averageHeight /= 6;

    double ushapeScore = 0;
    for (int x = 1; x <= 6; ++x) {
        static const int DIFF[] = { 0, 3, 0, -1, -1, 0, 3, 0 };
        ushapeScore -= std::abs((fieldBeforeRensa.height(x) - averageHeight) - DIFF[x]);
    }
    maxScore += 60 * ushapeScore;

#if 1
    // VALLEY
    for (int x = 1; x <= 6; ++x) {
        if (fieldBeforeRensa.valleyDepth(x) >= 4) {
            maxScore -= 200;
        }
    }
    // RIDGE
    for (int x = 1; x <= 6; ++x) {
        if (fieldBeforeRensa.ridgeHeight(x) >= 4) {
            maxScore -= 200;
        }
    }
#endif

    return std::make_pair(maxScore, maxChains);
}

SearchResult run(const std::vector<State>& initialStates, KumipuyoSeq seq, int maxSearchTurns,
                 std::mutex& mu)
{
    SearchResult result;

    std::vector<State> currentStates(initialStates);

    int maxOverallFiredChains = 0;
    int maxOverallFiredScore = 0;

    std::vector<State> nextStates;
    nextStates.reserve(100000);

    std::vector<double> time(std::max(maxSearchTurns, 10));

    double beginTime = currentTime();

    for (int turn = 3; turn < maxSearchTurns; ++turn) {
        time[turn] = currentTime();

        seq.dropFront();

        std::unordered_set<size_t> visited;

        int maxFiredScore = 0;
        int maxFiredRensa = 0;

        for (const State& s : currentStates) {
            Plan::iterateAvailablePlans(s.field, seq, 1, [&](const RefPlan& plan) {
                const CoreField& fieldBeforeRensa = plan.field();
                if (!visited.insert(fieldBeforeRensa.hash()).second)
                    return;

                int total_frames = s.total_frames + plan.totalFrames() + FRAMES_PREPARING_NEXT;

                if (plan.isRensaPlan()) {
                    maxOverallFiredScore = std::max(maxOverallFiredScore, plan.rensaResult().score);
                    maxOverallFiredChains = std::max(maxOverallFiredChains, plan.rensaResult().chains);

                    maxFiredScore = std::max(maxFiredScore, plan.rensaResult().score);
                    maxFiredRensa = std::max(maxFiredRensa, plan.rensaResult().chains);

                    nextStates.emplace_back(fieldBeforeRensa, s.firstDecision, plan.rensaResult().chains,
                                            plan.rensaResult().chains, total_frames);
                    return;
                }

                double maxScore;
                int maxChains;
                std::tie(maxScore, maxChains) = evalSuperLight(fieldBeforeRensa);
                nextStates.emplace_back(plan.field(), s.firstDecision, maxScore, maxChains, total_frames);
            });
        }

        std::sort(nextStates.begin(), nextStates.end(), std::greater<State>());

        int beamWidth = FLAGS_beam_width;
        if (turn <= 6) {
            beamWidth = 22 * 22;
        }

        if (nextStates.size() > static_cast<size_t>(beamWidth)) {
            nextStates.erase(nextStates.begin() + beamWidth, nextStates.end());
        }
        std::swap(currentStates, nextStates);
        nextStates.clear();

        if (false) {
            cout << "turn=" << turn
                 << " score=" << currentStates.front().stateScore
                 << " chains=" << currentStates.front().maxChains
                 << " first=" << currentStates.front().firstDecision
                 << " : fired_score=" << maxFiredScore
                 << " fired_rensa=" << maxFiredRensa
                 << endl;

            std::map<Decision, int> m;
            for (const auto& s : currentStates) {
                m[s.firstDecision] += 1;
            }
            cout << "decision kind = " << m.size() << endl;
            for (const auto& entry : m) {
                cout << entry.first << " " << entry.second << endl;
            }

            FieldPrettyPrinter::print(currentStates.front().field.toPlainField(), KumipuyoSeq());
        }
    }

    double endTime = currentTime();

    if (false) {
        lock_guard<mutex> lock(mu);
        if (!currentStates.empty()) {
            cout << "FIRED_CHAINS=" << maxOverallFiredChains
                 << " FIRED_SCORE=" << maxOverallFiredScore
                 << " DECISION=" << currentStates.front().firstDecision
                 << " TIME=" << (endTime - beginTime)
                 << " TURN4_TIME=" << (time[4] - beginTime)
                 << " TURN5_TIME=" << (time[5] - beginTime)
                 << " TURN6_TIME=" << (time[6] - beginTime) << endl;

        } else {
            cout << "EMPTY!" << endl;
            result.maxChains = -1;
            return result;
        }
    }

    result.maxChains = maxOverallFiredChains;
    result.firstDecisions.insert(currentStates.front().firstDecision);
    return result;
}

} // anonymous namespace

DropDecision BeamThinker::think(int /*frameId*/, const CoreField& field, const KumipuyoSeq& seq,
                                const PlayerState& /*me*/, const PlayerState& /*enemy*/, bool /*fast*/) const
{
    // If large enough, fire.
    if (true) {
        Decision tmpd;
        Plan::iterateAvailablePlans(field, seq, 2, [&](const RefPlan& plan) {
            if (plan.isRensaPlan() && plan.rensaResult().chains >= 14)
                tmpd = plan.firstDecision();
        });
        if (tmpd.isValid()) {
            return DropDecision(tmpd);
        }
    }

    // Decision -> max chains
    std::map<Decision, int> score;

    // Make initial states (the first move).
    std::vector<State> currentStates;
    Plan::iterateAvailablePlans(field, seq, 1, [&](const RefPlan& plan) {
        currentStates.emplace_back(plan.field(), plan.firstDecision(), 0, 0, plan.totalFrames());

        // Don't put 11th if rensa is not too good.
        if (plan.field().height(3) >= 11) {
            score[plan.firstDecision()] -= 100;
        }
    });

    // The second move.
    std::vector<State> nextStates;
    KumipuyoSeq subSeq = seq.subsequence(1);
    for (const auto& s : currentStates) {
        Plan::iterateAvailablePlans(s.field, subSeq, 1, [&](const RefPlan& plan) {
            int total_frames = s.total_frames + plan.totalFrames() + FRAMES_PREPARING_NEXT;
            nextStates.emplace_back(plan.field(), s.firstDecision, 0, 0, total_frames);
        });
    }

    WaitGroup wg;
    std::mutex mu;

    const int maxSearchTurns = std::min(FLAGS_beam_depth, (78 - field.countPuyos()) / 2 + 4);
#if 0
    cout << "maxSearchTurns = " << maxSearchTurns << endl;
#endif

    for (int k = 0; k < FLAGS_beam_num; ++k) {
        wg.add(1);

        executor_->submit([&]() {
            KumipuyoSeq tmpSeq(seq.subsequence(2));
            tmpSeq.append(KumipuyoSeqGenerator::generateRandomSequence(40));

            SearchResult searchResult = run(nextStates, tmpSeq, maxSearchTurns, mu_);

            {
                lock_guard<mutex> lk(mu);
                for (const auto& d : searchResult.firstDecisions) {
                    score[d] += searchResult.maxChains;
                }
            }
            // wg.done() should be called after releasing mu, because of deadlock.
            wg.done();
        });
    }

    wg.waitUntilDone();

    Decision d;
    int s = 0;
    for (const auto& entry : score) {
        if (s < entry.second) {
            s = entry.second;
            d = entry.first;
        }
    }

    return DropDecision(d, "BY BEAM");
}
