#include <iostream>
#include <unordered_set>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/algorithm/plan.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/algorithm/rensa_detector.h"
#include "core/core_field.h"
#include "core/decision.h"
#include "core/field_constant.h"
#include "core/field_pretty_printer.h"
#include "core/kumipuyo_seq_generator.h"
#include "core/pattern/decision_book.h"

#include "evaluator.h"
#include "pattern_book.h"
#include "pattern_rensa_detector.h"
#include "score_collector.h"
#include "shape_evaluator.h"

DEFINE_string(feature, "feature.toml", "the path to feature parameter");
DEFINE_string(decision_book, SRC_DIR "/cpu/mayah/decision.toml", "the path to decision book");
DEFINE_string(pattern_book, SRC_DIR "/cpu/mayah/pattern.toml", "the path to pattern book");

using namespace std;

const int BEAM_WIDTH = 1000;

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

class BeamMayahAI {
public:
    BeamMayahAI();

    int run(const KumipuyoSeq&);

private:
    EvaluationParameterMap evaluationParameterMap_;
    DecisionBook decisionBook_;
    PatternBook patternBook_;
};

BeamMayahAI::BeamMayahAI()
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

int BeamMayahAI::run(const KumipuyoSeq& originalSeq)
{
    int overallMaxFiredRensa = 0;

    KumipuyoSeq seq(originalSeq);

    vector<State> currentStates;
    Plan::iterateAvailablePlans(CoreField(), seq, 1, [&](const RefPlan& plan) {
        currentStates.emplace_back(plan.field(), plan.firstDecision(), 0, 0);
    });

    vector<State> nextStates;
    nextStates.reserve(100000);
    for (int turn = 1; turn < 40; ++turn) {
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
                    overallMaxFiredRensa = std::max(overallMaxFiredRensa, plan.rensaResult().chains);
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
                    const double possibility = PuyoPossibility::possibility(necessaryPuyoSet, std::max(0, numReachableSpace));
                    const double virtualRensaScore = rensaResult.score * possibility;

                    SimpleRensaScoreCollector rensaScoreCollector(sc.mainRensaParamSet(), sc.sideRensaParamSet());
                    RensaEvaluator<SimpleRensaScoreCollector> rensaEvaluator(patternBook_, &rensaScoreCollector);

                    rensaEvaluator.evalRensaRidgeHeight(complementedField);
                    rensaEvaluator.evalRensaValleyDepth(complementedField);
                    rensaEvaluator.evalRensaFieldUShape(complementedField);
                    rensaEvaluator.evalRensaIgnitionHeightFeature(complementedField, ignitionPuyoBits);
                    rensaEvaluator.evalRensaChainFeature(rensaResult, necessaryPuyoSet);
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
        if (nextStates.size() > BEAM_WIDTH) {
            nextStates.erase(nextStates.begin() + BEAM_WIDTH, nextStates.end());
        }
        std::swap(currentStates, nextStates);
        nextStates.clear();

        cout << "turn=" << turn
             << " score=" << currentStates.front().stateScore
             << " chains=" << currentStates.front().maxChains
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

#if 1
        FieldPrettyPrinter::print(currentStates.front().field.toPlainField(), KumipuyoSeq());
#endif
    }

    return overallMaxFiredRensa;
}

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();
    PuyoPossibility::initialize();

    BeamMayahAI ai;
    map<int, int> rensa;
    for (int i = 0; i < 100; ++i) {
        KumipuyoSeq seq = KumipuyoSeqGenerator::generateACPuyo2Sequence();
        int r = ai.run(seq);
        rensa[r] += 1;

        cout << "MAX FIRED RENSA : " << r << endl;
    }

    for (const auto& entry : rensa) {
        cout << entry.first << " -> " << entry.second << endl;
    }

    return 0;
}
