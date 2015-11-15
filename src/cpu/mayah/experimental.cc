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

DECLARE_string(feature);
DECLARE_string(decision_book);
DECLARE_string(pattern_book);

DEFINE_int32(beam_width, 400, "beam width");
DEFINE_int32(beam_depth, 40, "beam depth");
DEFINE_int32(beam_num, 8, "beam iteration number");

using namespace std;

struct State {
    State(const CoreField& field, const Decision& firstDecision, double stateScore, int maxChains) :
        field(field), firstDecision(firstDecision), stateScore(stateScore), maxChains(maxChains) {}

    friend bool operator<(const State& lhs, const State& rhs) { return lhs.stateScore < rhs.stateScore; }
    friend bool operator>(const State& lhs, const State& rhs) { return lhs.stateScore > rhs.stateScore; }

    CoreField field;
    Decision firstDecision;
    double stateScore = 0;
    int maxChains = 0;
};

struct SearchResult {
    set<Decision> firstDecisions;
    int maxChains = 0;
};

class BeamMayahAI : public AI {
public:
    BeamMayahAI(int argc, char* argv[]);

    DropDecision think(int frameId, const CoreField&, const KumipuyoSeq&,
                       const PlayerState& me, const PlayerState& enemy, bool fast) const override;

    SearchResult run(const vector<State>& initialStates, KumipuyoSeq, int maxSearchTurns) const;
    pair<double, int> eval(const CoreField& fieldBeforeRensa, int depth) const;
    pair<double, int> evalLight(const CoreField& fieldBeforeRensa) const;
    pair<double, int> evalSuperLight(const CoreField& fieldBeforeRensa) const;

private:
    EvaluationParameterMap evaluationParameterMap_;
    DecisionBook decisionBook_;
    PatternBook patternBook_;
    unique_ptr<Executor> executor_;

    mutable mutex mu_; // for cout
};

BeamMayahAI::BeamMayahAI(int argc, char* argv[]) :
    AI(argc, argv, "mayah-beam"),
    executor_(Executor::makeDefaultExecutor())
{
    CHECK(decisionBook_.load(FLAGS_decision_book));
    CHECK(patternBook_.load(FLAGS_pattern_book));

    if (evaluationParameterMap_.load(FLAGS_feature))
        return;

    // When not found, we try to load from the source directory.
    std::string filename = string(SRC_DIR) + "/cpu/mayah/" + FLAGS_feature;
    if (evaluationParameterMap_.load(filename))
        return;
}

DropDecision BeamMayahAI::think(int /*frameId*/, const CoreField& field, const KumipuyoSeq& seq,
                                const PlayerState& /*me*/, const PlayerState& /*enemy*/, bool /*fast*/) const
{
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

    if (true) {
        Decision d = decisionBook_.nextDecision(field, seq);
        if (d.isValid()) {
            return DropDecision(d);
        }
    }

    // Make initial states.
    vector<State> currentStates;
    Plan::iterateAvailablePlans(field, seq, 1, [&](const RefPlan& plan) {
        currentStates.emplace_back(plan.field(), plan.firstDecision(), 0, 0);
    });

    vector<State> nextStates;
    KumipuyoSeq subSeq = seq.subsequence(1);
    for (const auto& s : currentStates) {
        Plan::iterateAvailablePlans(s.field, subSeq, 1, [&](const RefPlan& plan) {
            nextStates.emplace_back(plan.field(), s.firstDecision, 0, 0);
        });
    }

    // Decision -> max chains
    map<Decision, int> score;

    WaitGroup wg;
    mutex mu;

    const int maxSearchTurns = std::max(5, min(FLAGS_beam_depth, std::max(seq.size(), (72 - field.countPuyos()) / 2)));
    cout << "maxSearchTurns = " << maxSearchTurns << endl;

    for (int k = 0; k < FLAGS_beam_num; ++k) {
        wg.add(1);

        executor_->submit([&]() {
            KumipuyoSeq tmpSeq(seq.subsequence(2));
            tmpSeq.append(KumipuyoSeqGenerator::generateRandomSequence(40));

            SearchResult searchResult = run(nextStates, tmpSeq, maxSearchTurns);

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

    return DropDecision(d, "");
}

SearchResult BeamMayahAI::run(const vector<State>& initialStates, KumipuyoSeq seq, int maxSearchTurns) const
{
    double beginTime = currentTime();

    SearchResult result;

    vector<State> currentStates(initialStates);

    int maxOverallFiredChains = 0;
    int maxOverallFiredScore = 0;

    vector<State> nextStates;
    nextStates.reserve(100000);

    vector<double> time(std::max(maxSearchTurns, 10));

    for (int turn = 3; turn < maxSearchTurns; ++turn) {

#if 0
        {
            bool found = false;
            Decision fd = currentStates.front().firstDecision;
            for (const auto& s : currentStates) {
                if (s.firstDecision != fd) {
                    found = true;
                    break;
                }
            }

            if (!found)
                break;
        }

#endif
        time[turn] = currentTime();

        seq.dropFront();

        unordered_set<size_t> visited;

        int maxFiredScore = 0;
        int maxFiredRensa = 0;

        for (const State& s : currentStates) {
            Plan::iterateAvailablePlans(s.field, seq, 1, [&](const RefPlan& plan) {
                const CoreField& fieldBeforeRensa = plan.field();
                if (!visited.insert(fieldBeforeRensa.hash()).second)
                    return;

                if (plan.isRensaPlan()) {
                    maxOverallFiredScore = std::max(maxOverallFiredScore, plan.rensaResult().score);
                    maxOverallFiredChains = std::max(maxOverallFiredChains, plan.rensaResult().chains);

                    maxFiredScore = std::max(maxFiredScore, plan.rensaResult().score);
                    maxFiredRensa = std::max(maxFiredRensa, plan.rensaResult().chains);
                    nextStates.emplace_back(fieldBeforeRensa, s.firstDecision, plan.rensaResult().chains, plan.rensaResult().chains);
                    return;
                }

                double maxScore;
                int maxChains;
                if (turn >= 6) {
                    std::tie(maxScore, maxChains) = evalSuperLight(fieldBeforeRensa);
                } else if (turn >= 4) {
                    std::tie(maxScore, maxChains) = eval(fieldBeforeRensa, 1);
                } else {
                    std::tie(maxScore, maxChains) = eval(fieldBeforeRensa, 2);
                }

                nextStates.emplace_back(plan.field(), s.firstDecision, maxScore, maxChains);
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

#if 0
        cout << "turn=" << turn
             << " score=" << currentStates.front().stateScore
             << " chains=" << currentStates.front().maxChains
             << " first=" << currentStates.front().firstDecision
             << " : fired_score=" << maxFiredScore
             << " fired_rensa=" << maxFiredRensa
             << endl;

        map<Decision, int> m;
        for (const auto& s : currentStates) {
            m[s.firstDecision] += 1;
        }
        cout << "decision kind = " << m.size() << endl;
        for (const auto& entry : m) {
            cout << entry.first << " " << entry.second << endl;
        }

        FieldPrettyPrinter::print(currentStates.front().field.toPlainField(), KumipuyoSeq());
#endif

    }

    double endTime = currentTime();

    {
        lock_guard<mutex> lock(mu_);
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

pair<double, int> BeamMayahAI::evalLight(const CoreField& fieldBeforeRensa) const
{
    int maxChains = 0;
    auto callback = [&](const CoreField& /*fieldAfterRensa*/,
                        const RensaResult& rensaResult,
                        const ColumnPuyoList& /*puyosToComplement*/,
                        PuyoColor /*firePuyoColor*/,
                        const std::string& /*patternName*/,
                        double /*patternScore*/) {
        maxChains = std::max(maxChains, rensaResult.chains);
    };
    PatternRensaDetector(patternBook_, fieldBeforeRensa, callback).iteratePossibleRensas(1);

    double maxScore = 0;
    maxScore += maxChains * 1000;

    double averageHeight = 0;
    for (int x = 1; x <= 6; ++x)
        averageHeight += fieldBeforeRensa.height(x);
    averageHeight /= 6;

    double ushapeScore = 0;
    for (int x = 1; x <= 6; ++x) {
        static const int DIFF[] = { 0, 2, 0, -2, -2, 0, 2, 0 };
        ushapeScore -= std::abs((fieldBeforeRensa.height(x) - averageHeight) - DIFF[x]);
    }
    maxScore += 60 * ushapeScore;

    return make_pair(maxScore, maxChains);
}

pair<double, int> BeamMayahAI::evalSuperLight(const CoreField& fieldBeforeRensa) const
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
        static const int DIFF[] = { 0, 2, 0, -2, -2, 0, 2, 0 };
        ushapeScore -= std::abs((fieldBeforeRensa.height(x) - averageHeight) - DIFF[x]);
    }
    maxScore += 60 * ushapeScore;

    return make_pair(maxScore, maxChains);
}

pair<double, int> BeamMayahAI::eval(const CoreField& fieldBeforeRensa, int depth) const
{
    SimpleScoreCollector sc(evaluationParameterMap_);
    ShapeEvaluator<SimpleScoreCollector>(&sc).eval(fieldBeforeRensa);

    EvaluationMode mode = EvaluationMode::MIDDLE;
    {
        // const int INITIAL_THRESHOLD = 16;
        const int EARLY_THRESHOLD = 24;
        const int MIDDLE_THRESHOLD = 36;
        // const int LATE_THRESHOLD = 54;

        int cnt = fieldBeforeRensa.countPuyos();
        if (cnt <= EARLY_THRESHOLD) {
            mode = EvaluationMode::EARLY;
        } else if (cnt <= MIDDLE_THRESHOLD) {
            mode = EvaluationMode::MIDDLE;
        } else {
            mode = EvaluationMode::LATE;
        }
    }

    const double shapeScore = sc.collectedScore().score(mode);

    const int numReachableSpace = fieldBeforeRensa.countConnectedPuyos(3, 12);

    double maxScore = 0;
    int maxChains = 0;
    auto callback = [&](const CoreField& fieldAfterRensa,
                        const RensaResult& rensaResult,
                        const ColumnPuyoList& puyosToComplement,
                        PuyoColor /*firePuyoColor*/,
                        const std::string& /*patternName*/,
                        double patternScore) {
        maxChains = std::max(maxChains, rensaResult.chains);

        CoreField complementedField(fieldBeforeRensa);
        if (!complementedField.dropPuyoList(puyosToComplement))
            return;

        const PuyoSet necessaryPuyoSet(puyosToComplement);
        const double possibility = PuyoSetProbability::instanceSlow()->possibility(necessaryPuyoSet, std::max(0, numReachableSpace));
        const double virtualRensaScore = rensaResult.score * possibility;

        SimpleRensaScoreCollector rensaScoreCollector(sc.mainRensaParamSet(), sc.sideRensaParamSet());
        RensaEvaluator<SimpleRensaScoreCollector> rensaEvaluator(patternBook_, &rensaScoreCollector);
        rensaEvaluator.eval(complementedField, fieldBeforeRensa, fieldAfterRensa, rensaResult,
                            puyosToComplement, patternScore, virtualRensaScore);

        const double rensaScore = rensaScoreCollector.mainRensaScore().score(EvaluationMode::MIDDLE);

        double score = rensaScore + shapeScore;
        maxScore = std::max(maxScore, score);
    };
    PatternRensaDetector(patternBook_, fieldBeforeRensa, callback).iteratePossibleRensas(depth);

    return make_pair(maxScore, maxChains);
}

// ----------------------------------------------------------------------

class MixedMayahAI : public AI {
public:
    MixedMayahAI(int argc, char* argv[]);

    DropDecision think(int frameId, const CoreField&, const KumipuyoSeq&,
                       const PlayerState& me, const PlayerState& enemy, bool fast) const override;

private:
    DebuggableMayahAI ai_;
    BeamMayahAI beamAi_;
};

MixedMayahAI::MixedMayahAI(int argc, char* argv[]) :
    AI(argc, argv, "mixed"),
    ai_(argc, argv),
    beamAi_(argc, argv)
{
    ai_.setUsesRensaHandTree(false);
    ai_.removeNontokopuyoParameter();
}

DropDecision MixedMayahAI::think(int frameId, const CoreField& field, const KumipuyoSeq& seq,
                                 const PlayerState& me, const PlayerState& enemy, bool fast) const
{
    // if (field.countPuyos() <= 24)
    //     return ai_.think(frameId, field, seq, me, enemy, fast);
    // else
        return beamAi_.think(frameId, field, seq, me, enemy, fast);
}

// ----------------------------------------------------------------------

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    (void)PuyoSetProbability::instanceSlow();
    (void)ColumnPuyoListProbability::instanceSlow();

    // unique_ptr<BeamMayahAI> ai(new BeamMayahAI(argc, argv));
    unique_ptr<MixedMayahAI> ai(new MixedMayahAI(argc, argv));

#if 0
    for (int i = 0; i < 50; ++i) {
        KumipuyoSeq seq = KumipuyoSeqGenerator::generateACPuyo2Sequence();
        ai->run(CoreField(), seq);
    }
#else
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

    vector<int> scores;
    vector<int> rensas;

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
        int average = sum / N;

        cout << "        N = " << N << endl;
        cout << "      min = " << *min_element(scores.begin(), scores.end()) << endl;
        cout << "      max = " << *max_element(scores.begin(), scores.end()) << endl;
        cout << "  average = " << average << endl;

        for (int i = 10; i <= 90; i += 10)
            cout << "      " << i << "% = " << scores[i] << endl;
        cout << "     num8 = " << num8 << endl;
        cout << "     num9 = " << num9 << endl;
        cout << "    num10 = " << num10 << endl;

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
    }

#endif
    return 0;
}
