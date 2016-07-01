#include "pattern_thinker.h"

#include <vector>

#include "decision_planner.h"
#include "score_collector.h"

using namespace std;

PatternThinker::PatternThinker(const EvaluationParameterMap& evaluationParameterMap,
                               const DecisionBook& decisionBook,
                               const PatternBook& patternBook,
                               Executor* executor) :
    evaluationParameterMap_(evaluationParameterMap),
    decisionBook_( decisionBook),
    patternBook_(patternBook),
    executor_(executor)
{
}

DropDecision PatternThinker::think(int frame_id, const CoreField& field, const KumipuyoSeq& kumipuyo_seq,
                                   const PlayerState& me, const PlayerState& enemy,
                                   const GazeResult& gazeResult, bool fast,
                                   bool usesDecisionBook, bool usesRensaHandTree) const
{
    int depth;
    int iteration;
    if (fast) {
        depth = FAST_DEPTH;
        iteration = FAST_NUM_ITERATION;
    } else {
        depth = DEFAULT_DEPTH;
        iteration = DEFAULT_NUM_ITERATION;
    }

    ThoughtResult thoughtResult = thinkPlan(frame_id, field, kumipuyo_seq, me, enemy, depth, iteration, gazeResult, fast, usesDecisionBook, usesRensaHandTree);

    const Plan& plan = thoughtResult.plan;
    if (plan.decisions().empty())
        return DropDecision(Decision(3, 0), thoughtResult.message);
    return DropDecision(plan.decisions().front(), thoughtResult.message);
}

ThoughtResult PatternThinker::thinkPlan(int frameId, const CoreField& field, const KumipuyoSeq& kumipuyoSeq,
                                        const PlayerState& me, const PlayerState& enemy,
                                        int depth, int maxIteration,
                                        const GazeResult& gazeResult,
                                        bool fast,
                                        bool usesDecisionBook, bool usesRensaHandTree,
                                        vector<Decision>* specifiedDecisions) const
{
    // TODO(mayah): Do we need field and kumipuyoSeq?
    // CHECK(field, me.field);
    // CHECK(kumipuyoSeq, me.kumipuyoSeq);

    LOG(INFO) << "\n" << field.toDebugString() << "\n" << kumipuyoSeq.toString();
    if (VLOG_IS_ON(1)) {
        VLOG(1) << "\n"
                << "----------------------------------------------------------------------" << endl
                << "think frameId = " << frameId << endl
                << "my ojama: fixed=" << me.fixedOjama << " pending=" << me.pendingOjama
                << " total=" << me.totalOjama(enemy) << endl
                << "enemy ojama: fixed=" << enemy.fixedOjama << " pending=" << enemy.pendingOjama
                << " total=" << enemy.totalOjama(me) << endl
                << "enemy rensa: ending = " << (enemy.isRensaOngoing() ? enemy.rensaFinishingFrameId() : 0) << endl
                << "enemy gaze result: " << gazeResult.toRensaInfoString()
                << "----------------------------------------------------------------------" << endl;
    }

    if (kumipuyoSeq.size() < 2) {
        LOG(ERROR) << "The size of kumipuyoSeq is " << kumipuyoSeq.size() << ", which is < 2.";
        // TODO(mayah): This shouldn't happen. However, this happens on wii.
        CoreField cf(field);
        Decision d(1, 1);
        vector<Decision> decisions { d };

        ThoughtResult tr(Plan(cf, decisions, RensaResult(), 0, 0, 0, 0, 0, 0, 0, false),
                         0.0, 0.0, MidEvalResult(), "Invalid KumipuyoSeq.");
        return tr;
    }

    if (usesDecisionBook && !enemy.hasZenkeshi) {
        Decision d = decisionBook_.nextDecision(field, kumipuyoSeq);
        if (d.isValid()) {
            CoreField cf(field);
            cf.dropKumipuyo(d, kumipuyoSeq.front());
            vector<Decision> decisions { d };

            ThoughtResult tr(Plan(cf, decisions, RensaResult(), 0, 0, 0, 0, 0, 0, 0, false),
                             0.0, 0.0, MidEvalResult(), "BY DECISION BOOK");
            return tr;
        }
    }

    Plan bestPlan;
    double bestScore = -100000000.0;
    MidEvalResult bestMidEvalResult;

    Plan bestRensaPlan;
    int bestRensaScore = 0;
    int bestRensaFrames = 0;
    MidEvalResult bestRensaMidEvalResult;

    int bestVirtualRensaScore = 0;

    bool ojamaFallen = false;

    mutex mu;
    auto evalRefPlan = [&, this, frameId, maxIteration](const RefPlan& plan, const MidEvalResult& midEvalResult) {
        KumipuyoSeq restSeq(kumipuyoSeq.subsequence(plan.decisions().size()));
        // Here, we iterate enemy's possible rensa.
        EvalResult evalResult = eval(plan, restSeq, frameId, maxIteration, me, enemy, midEvalResult, fast, usesRensaHandTree, gazeResult);
        Plan evaledPlan = plan.toPlan();

        // Hmm, it looks weaker if we search this...
#if 0
        if (!gazeResult.feasibleRensaHandTree().nodes().empty()) {
            const RensaHandNode& node = gazeResult.feasibleRensaHandTree().node(0);
            for (const auto& edge : node.edges()) {
                int frameIdRensaFinished = gazeResult.frameIdToStartNextMove() + edge.rensaHand().totalFrames();
                if (plan.framesToIgnite() < frameIdRensaFinished)
                    continue;

                int numOjama = edge.rensaHand().score() / 70;
                numOjama -= enemy.totalOjama(me);
                if (numOjama < 0)
                    continue;

                int lines = std::min(5, (me.totalOjama(enemy) + numOjama + 4) / 6);
                if (lines == 0)
                    continue;

                // TODO(mayah): p must be more correct.
                // When enemy has ZENKESHI, p should consider it.
                // Enemy PlayerState should also be updated.

                Plan p = plan.toPlan();
                int ojamaFrames = p.mutableField()->fallOjama(lines);
                p.setLastDropFrames(p.lastDropFrames() + ojamaFrames);

                // TODO(mayah): Instead of gazeResult, we need to use edge.tree().
                EvalResult result = eval(RefPlan(p), restSeq, frameId, maxIteration, me, enemy, midEvalResult, fast, usesRensaHandTree, gazeResult);
                if (result.score() < evalResult.score()) {
                    evalResult = result;
                    evaledPlan = p;
                }
            }
        }
#endif

        VLOG(1) << toString(plan.decisions())
                << ": eval=" << evalResult.score()
                << " pscore=" << plan.score()
                << " vscore=" << evalResult.maxVirtualScore();

        lock_guard<mutex> lock(mu);

        if (evaledPlan.fallenOjama() > 0)
            ojamaFallen = true;

        if (bestScore < evalResult.score()) {
            bestScore = evalResult.score();
            bestPlan = evaledPlan;
            bestMidEvalResult = midEvalResult;
        }

        if (bestVirtualRensaScore < evalResult.maxVirtualScore()) {
            bestVirtualRensaScore = evalResult.maxVirtualScore();
        }

        if (bestRensaScore < evaledPlan.score() || (bestRensaScore == evaledPlan.score() && bestRensaFrames > evaledPlan.totalFrames())) {
            bestRensaScore = evaledPlan.score();
            bestRensaFrames = evaledPlan.totalFrames();
            bestRensaPlan = evaledPlan;
            bestRensaMidEvalResult = midEvalResult;
        }
    };
    auto evalMidEval = [&](const RefPlan& plan) {
        return midEval(plan, field, kumipuyoSeq.subsequence(plan.decisions().size()),
                       frameId, maxIteration, me, enemy, gazeResult, usesRensaHandTree);
    };

    DecisionPlanner<MidEvalResult> planner(executor_, evalMidEval, evalRefPlan);
    if (specifiedDecisions)
        planner.setSpecifiedDecisions(*specifiedDecisions);
    planner.iterate(frameId, field, kumipuyoSeq, me, enemy, depth);

    if (!ojamaFallen && bestVirtualRensaScore < bestRensaScore) {
        std::string message = makeMessageFrom(frameId, kumipuyoSeq, maxIteration,
                                              me, enemy,
                                              bestRensaMidEvalResult, gazeResult,
                                              bestRensaPlan, bestRensaScore, bestVirtualRensaScore,
                                              true, fast, usesRensaHandTree);
        return ThoughtResult(bestRensaPlan, bestRensaScore, bestVirtualRensaScore, bestRensaMidEvalResult, message);
    } else {
        std::string message = makeMessageFrom(frameId, kumipuyoSeq, maxIteration,
                                              me, enemy,
                                              bestMidEvalResult, gazeResult,
                                              bestPlan, bestRensaScore, bestVirtualRensaScore,
                                              false, fast, usesRensaHandTree);
        return ThoughtResult(bestPlan, bestRensaScore, bestVirtualRensaScore, bestMidEvalResult, message);
    }
}

MidEvalResult PatternThinker::midEval(const RefPlan& plan,
                                      const CoreField& currentField,
                                      const KumipuyoSeq& restSeq,
                                      int currentFrameId, int maxIteration,
                                      const PlayerState& me,
                                      const PlayerState& enemy,
                                      const GazeResult& gazeResult,
                                      bool usesRensaHandTree) const
{
    SimpleScoreCollector sc(evaluationParameterMap_);
    Evaluator<SimpleScoreCollector> evaluator(patternBook_, &sc);

    // MidEval always sets 'fast'.
    evaluator.eval(plan, restSeq, currentFrameId, maxIteration, me, enemy, MidEvalResult(), true, usesRensaHandTree, gazeResult);

    MidEvaluator midEvaluator(patternBook_);
    const CollectedSimpleScore& simpleScore = sc.collectedScore();
    return midEvaluator.eval(plan, currentField, simpleScore.score(sc.collectedCoef()));
}

EvalResult PatternThinker::eval(const RefPlan& plan,
                                const KumipuyoSeq& restSeq,
                                int currentFrameId, int maxIteration,
                                const PlayerState& me, const PlayerState& enemy,
                                const MidEvalResult& midEvalResult,
                                bool fast, bool usesRensaHandTree,
                                const GazeResult& gazeResult) const
{
    SimpleScoreCollector sc(evaluationParameterMap_);
    Evaluator<SimpleScoreCollector> evaluator(patternBook_, &sc);
    evaluator.eval(plan, restSeq, currentFrameId, maxIteration, me, enemy, midEvalResult, fast, usesRensaHandTree, gazeResult);

    const CollectedSimpleScore& simpleScore = sc.collectedScore();
    return EvalResult(simpleScore.score(sc.collectedCoef()), sc.estimatedRensaScore());
}

CollectedFeatureCoefScore PatternThinker::evalWithCollectingFeature(const RefPlan& plan,
                                                                    const KumipuyoSeq& restSeq,
                                                                    int currentFrameId,
                                                                    int maxIteration,
                                                                    const PlayerState& me,
                                                                    const PlayerState& enemy,
                                                                    const MidEvalResult& midEvalResult,
                                                                    bool fast,
                                                                    bool usesRensaHandTree,
                                                                    const GazeResult& gazeResult) const
{
    FeatureScoreCollector sc(evaluationParameterMap_);
    Evaluator<FeatureScoreCollector> evaluator(patternBook_, &sc);
    evaluator.eval(plan, restSeq, currentFrameId, maxIteration, me, enemy, midEvalResult, fast, usesRensaHandTree, gazeResult);

    return CollectedFeatureCoefScore(sc.collectedCoef(), sc.collectedScore());
}

std::string PatternThinker::makeMessageFrom(int frameId, const KumipuyoSeq& kumipuyoSeq, int maxIteration,
                                            const PlayerState& me, const PlayerState& enemy,
                                            const MidEvalResult& midEvalResult,
                                            const GazeResult& gazeResult,
                                            const Plan& plan, double rensaScore, double virtualRensaScore,
                                            bool saturated, bool fast, bool usesRensaHandTree) const
{
    UNUSED_VARIABLE(kumipuyoSeq);

    if (plan.decisions().empty())
        return string("give up :-(");

    RefPlan refPlan(plan);
    CollectedFeatureCoefScore cf =
        evalWithCollectingFeature(refPlan, kumipuyoSeq.subsequence(refPlan.decisions().size()),
                                  frameId, maxIteration, me, enemy, midEvalResult, fast, usesRensaHandTree, gazeResult);


    stringstream ss;
    ss << "MODE ";
    for (const auto& mode : ALL_EVALUATION_MODES) {
        if (cf.coef(mode) > 0)
            ss << toString(mode) << "(" << cf.coef(mode) << ") ";
    }
    ss << "/ ";
    if (cf.moveScore().feature(STRATEGY_ZENKESHI) > 0)
        ss << "ZENKESHI / ";
    if (cf.moveScore().feature(STRATEGY_KILL) > 0)
        ss << "KILL / ";
    if (cf.moveScore().feature(STRATEGY_FIRE_SIDE_CHAIN_2_LARGE) > 0) {
        ss << "SIDE CHAIN LARGE (2) / ";
    } else if (cf.moveScore().feature(STRATEGY_FIRE_SIDE_CHAIN_2_MEDIUM) > 0) {
        ss << "SIDE CHAIN MEDIUM (2) / ";
    } else if (cf.moveScore().feature(STRATEGY_FIRE_SIDE_CHAIN_2_SMALL) > 0) {
        ss << "SIDE CHAIN SMALL (2) / ";
    } else if (cf.moveScore().feature(STRATEGY_FIRE_SIDE_CHAIN_3_LARGE) > 0) {
        ss << "SIDE CHAIN LARGE (3) / ";
    } else if (cf.moveScore().feature(STRATEGY_FIRE_SIDE_CHAIN_3_MEDIUM) > 0) {
        ss << "SIDE CHAIN MEDIUM (3) / ";
    } else if (cf.moveScore().feature(STRATEGY_FIRE_SIDE_CHAIN_3_SMALL) > 0) {
        ss << "SIDE CHAIN SMALL (3) / ";
    }
    if (cf.moveScore().feature(STRATEGY_TAIOU) > 0)
        ss << "TAIOU / ";
    if (cf.moveScore().feature(STRATEGY_TAIOU_RELUCTANT) > 0)
        ss << "TAIOU (RELUCTANT) / ";
    if (cf.moveScore().feature(STRATEGY_LARGE_ENOUGH) > 0)
        ss << "LARGE_ENOUGH / ";
    if (cf.moveScore().feature(STRATEGY_TSUBUSHI) > 0)
        ss << "TSUBUSHI / ";
    if (cf.mainRensaScore().feature(STRATEGY_SAISOKU) > 0)
        ss << "SAISOKU / ";
    else if (saturated)
        ss << "SATURATED / ";
    else if (cf.moveScore().feature(STRATEGY_SAKIUCHI) > 0)
        ss << "SAKIUCHI / ";
    if (cf.moveScore().feature(STRATEGY_ZENKESHI_CONSUME) > 0)
        ss << "USE_ZENKESHI / ";
    if (cf.moveScore().feature(STRATEGY_LAND_LEVELING) > 0)
        ss << "LEVELING / ";

    if (!cf.mainRensaScore().bookname.empty())
        ss << cf.mainRensaScore().bookname << " / ";

    ss << "D/I = " << plan.decisions().size() << "/" << maxIteration << " / ";
    ss << "SCORE = " << cf.score() << " / ";

    if (!cf.mainRensaScore().feature(MAX_CHAINS).empty()) {
        const vector<int>& vs = cf.mainRensaScore().feature(MAX_CHAINS);
        for (size_t i = 0; i < vs.size(); ++i)
            ss << "MAX CHAIN = " << vs[i] << " / ";
    }

    ss << "R/V SCORE=" << rensaScore << "/" << virtualRensaScore;

    ss << "\n";

    if (cf.moveScore().feature(HOLDING_SIDE_CHAIN_SMALL) > 0) {
        ss << "SIDE=SMALL";
    } else if (cf.moveScore().feature(HOLDING_SIDE_CHAIN_MEDIUM) > 0) {
        ss << "SIDE=MEDIUM";
    } else if (cf.moveScore().feature(HOLDING_SIDE_CHAIN_LARGE) > 0) {
        ss << "SIDE=LARGE";
    } else {
        ss << "SIDE=NONE";
    }
    ss << " / ";

    if (cf.moveScore().feature(KEEP_FAST_LARGER_THEN_ENEMY) > 0) {
        ss << "FAST_LARGE=OK / ";
    } else {
        ss << "FAST_LARGE=NG / ";
    }
    if (cf.moveScore().feature(KEEP_FAST_4_CHAIN) > 0) {
        ss << "FAST4=OK / ";
    } else {
        ss << "FAST4=NG / ";
    }
    if (cf.moveScore().feature(KEEP_FAST_6_CHAIN) > 0) {
        ss << "FAST6=OK / ";
    } else {
        ss << "FAST6=NG / ";
    }
    if (cf.moveScore().feature(KEEP_FAST_10_CHAIN) > 0) {
        ss << "FAST10=OK / ";
    } else {
        ss << "FAST10=NG / ";
    }

    ss << "HAND_TREE=" << cf.moveScore().feature(STRATEGY_RENSA_TREE);

    ss << "\n";

    return ss.str();
}
