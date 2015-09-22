#include <iostream>
#include <unordered_set>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/executor.h"
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
#include "core/probability/puyo_set_probability.h"
#include "solver/endless.h"
#include "solver/puyop.h"

#include "evaluator.h"
#include "pattern_book.h"
#include "pattern_rensa_detector.h"
#include "score_collector.h"
#include "shape_evaluator.h"

DEFINE_string(feature, "feature.toml", "the path to feature parameter");
DEFINE_string(decision_book, SRC_DIR "/cpu/mayah/decision.toml", "the path to decision book");
DEFINE_string(pattern_book, SRC_DIR "/cpu/mayah/pattern.toml", "the path to pattern book");

DEFINE_int32(initial_beam_width, 100, "");
DEFINE_int32(beam_width, 100, "beam width");
DEFINE_int32(beam_depth, 10, "beam depth");
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

    SearchResult run(const CoreField&, const KumipuyoSeq&) const;

private:
    EvaluationParameterMap evaluationParameterMap_;
    DecisionBook decisionBook_;
    PatternBook patternBook_;
    unique_ptr<Executor> executor_;
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
    // Decision -> max chains
    map<Decision, int> score;

    WaitGroup wg;
    mutex mu;

    for (int k = 0; k < FLAGS_beam_num; ++k) {
        wg.add(1);
        executor_->submit([&]() {
            SearchResult searchResult = run(field, seq);

            lock_guard<mutex> lk(mu);
            for (const auto& d : searchResult.firstDecisions) {
                score[d] += searchResult.maxChains;
            }
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

SearchResult BeamMayahAI::run(const CoreField& originalField, const KumipuyoSeq& originalSeq) const
{
    SearchResult result;

    KumipuyoSeq seq(originalSeq);
    seq.append(KumipuyoSeqGenerator::generateRandomSequence(40));

    vector<State> currentStates;
    Plan::iterateAvailablePlans(originalField, seq, 1, [&](const RefPlan& plan) {
        currentStates.emplace_back(plan.field(), plan.firstDecision(), 0, 0);
    });

    vector<State> nextStates;
    nextStates.reserve(100000);
    for (int turn = 1; turn < FLAGS_beam_depth; ++turn) {
        seq.dropFront();

        unordered_set<size_t> visited;

        int maxFiredScore = 0;
        int maxFiredRensa = 0;

        for (const State& s : currentStates) {
            vector<int> matchablePatternIds;
            for (size_t i = 0; i < patternBook_.size(); ++i) {
                const PatternBookField& pbf = patternBook_.patternBookField(i);
                if (pbf.ignitionColumn() == 0)
                    continue;
                if (pbf.isMatchable(s.field))
                    matchablePatternIds.push_back(static_cast<int>(i));
            }

            Plan::iterateAvailablePlans(s.field, seq, 1, [&](const RefPlan& plan) {
                const CoreField& fieldBeforeRensa = plan.field();
                const CoreField& field = plan.field();
                if (!visited.insert(field.hash()).second)
                    return;

                if (plan.isRensaPlan()) {
                    maxFiredScore = std::max(maxFiredScore, plan.rensaResult().score);
                    maxFiredRensa = std::max(maxFiredRensa, plan.rensaResult().chains);
#if 0
                    if (maxFiredRensa > result.maxChains) {
                        result.maxChains = maxFiredRensa;
                        result.firstDecisions.clear();
                        result.firstDecisions.insert(s.firstDecision);
                    } else if (maxFiredRensa == result.maxChains) {
                        result.firstDecisions.insert(s.firstDecision);
                    }
#endif
                    nextStates.emplace_back(field, s.firstDecision, plan.rensaResult().chains, plan.rensaResult().chains);
                    return;
                }

                SimpleScoreCollector sc(evaluationParameterMap_);
                ShapeEvaluator<SimpleScoreCollector>(&sc).eval(field);
                const double shapeScore = sc.collectedScore().score(EvaluationMode::MIDDLE);

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

                    FieldBits ignitionPuyoBits = complementedField.ignitionPuyoBits();

                    const PuyoSet necessaryPuyoSet(puyosToComplement);
                    const double possibility = PuyoSetProbability::instanceSlow()->possibility(necessaryPuyoSet, std::max(0, numReachableSpace));
                    const double virtualRensaScore = rensaResult.score * possibility;

                    SimpleRensaScoreCollector rensaScoreCollector(sc.mainRensaParamSet(), sc.sideRensaParamSet());
                    RensaEvaluator<SimpleRensaScoreCollector> rensaEvaluator(patternBook_, &rensaScoreCollector);

                    rensaEvaluator.evalRensaRidgeHeight(complementedField);
                    rensaEvaluator.evalRensaValleyDepth(complementedField);
                    rensaEvaluator.evalRensaFieldUShape(complementedField);
                    rensaEvaluator.evalRensaIgnitionHeightFeature(complementedField, ignitionPuyoBits);
                    rensaEvaluator.evalRensaChainFeature(rensaResult, puyosToComplement);
                    rensaEvaluator.evalRensaGarbage(fieldAfterRensa);
                    rensaEvaluator.evalPatternScore(puyosToComplement, patternScore, rensaResult.chains);
                    rensaEvaluator.evalFirePointTabooFeature(fieldBeforeRensa, ignitionPuyoBits); // fieldBeforeRensa is correct.
                    rensaEvaluator.evalRensaConnectionFeature(fieldAfterRensa);
                    rensaEvaluator.evalComplementationBias(puyosToComplement);
                    rensaEvaluator.evalRensaScore(rensaResult.score, virtualRensaScore);
                    // rensaEvaluator.evalRensaStrategy(plan, rensaResult, puyosToComplement, currentFrameId, me, enemy);

                    const double rensaScore = rensaScoreCollector.mainRensaScore().score(EvaluationMode::MIDDLE);

                    double score = rensaScore + shapeScore;
                    maxScore = std::max(maxScore, score);

                };
                PatternRensaDetector(patternBook_, field, callback).iteratePossibleRensas(matchablePatternIds, 2);

                nextStates.emplace_back(plan.field(), s.firstDecision, maxScore, maxChains);
            });
        }

        std::sort(nextStates.begin(), nextStates.end(), std::greater<State>());

        int beamWidth = FLAGS_beam_width;
        if (turn <= 2) {
            beamWidth = FLAGS_initial_beam_width;
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
#endif

#if 0
        FieldPrettyPrinter::print(currentStates.front().field.toPlainField(), KumipuyoSeq());
#endif

    }

    result.maxChains = 1;
    result.firstDecisions.insert(currentStates.front().firstDecision);
    return result;
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    unique_ptr<BeamMayahAI> ai(new BeamMayahAI(argc, argv));
    Endless endless(std::move(ai));
    endless.setVerbose(true);
    //endless.setVerbose(FLAGS_show_field);

    KumipuyoSeq seq = KumipuyoSeqGenerator::generateACPuyo2Sequence();
    EndlessResult result = endless.run(seq);

    cout << seq.toString() << endl;
    cout << makePuyopURL(seq, result.decisions) << endl;
    cout << "score = " << result.score << " rensa = " << result.maxRensa;
    if (result.zenkeshi)
        cout << " / ZENKESHI";
    cout << endl;

    return 0;
}
