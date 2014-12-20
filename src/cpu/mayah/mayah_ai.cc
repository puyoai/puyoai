#include "mayah_ai.h"

#include <iostream>
#include <sstream>

#include <gflags/gflags.h>

#include "base/time.h"
#include "base/wait_group.h"
#include "core/algorithm/plan.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/frame_request.h"

#include "opening_book.h"
#include "evaluation_parameter.h"
#include "evaluator.h"
#include "gazer.h"

DEFINE_string(feature, SRC_DIR "/cpu/mayah/feature.toml", "the path to feature parameter");
DEFINE_string(opening_book, SRC_DIR "/cpu/mayah/opening.toml", "the path to opening book");
DEFINE_string(decision_book, SRC_DIR "/cpu/mayah/decision.toml", "the path to decision book");
DEFINE_bool(use_advanced_next, false, "Use enemy's NEXT sequence also");

using namespace std;

MayahAI::MayahAI(int argc, char* argv[], Executor* executor) :
    AI(argc, argv, "mayah"),
    executor_(executor)
{
    setBehaviorRethinkAfterOpponentRensa(true);

    evaluationParameter_.reset(new EvaluationParameter(FLAGS_feature));
    CHECK(openingBook_.load(FLAGS_opening_book));
    CHECK(decisionBook_.load(FLAGS_decision_book));

    VLOG(1) << evaluationParameter_->toString();
    VLOG(1) << openingBook_.toString();

    google::FlushLogFiles(google::INFO);
}

MayahAI::~MayahAI()
{
}

void MayahAI::reloadParameter()
{
    evaluationParameter_.reset(new EvaluationParameter(FLAGS_feature));
}

DropDecision MayahAI::think(int frameId, const CoreField& f, const KumipuyoSeq& kumipuyoSeq,
                            const AdditionalThoughtInfo& additionalThoughtInfo, bool fast)
{
    int depth;
    int iteration;
    if (fast) {
      depth = MayahAI::DEFAULT_DEPTH;
      iteration = MayahAI::FAST_NUM_ITERATION;
    } else if (FLAGS_use_advanced_next) {
        if (kumipuyoSeq.size() >= 3) {
            depth = 3;
            iteration = 2;
        } else if (f.countPuyos() <= MIDDLE_THRESHOLD) {
            depth = 2;
            iteration = 3;
        } else {
            depth = 2;
            iteration = 2;
        }
    } else {
        depth = MayahAI::DEFAULT_DEPTH;
        iteration = MayahAI::DEFAULT_NUM_ITERATION;
    }

    ThoughtResult thoughtResult = thinkPlan(frameId, f, kumipuyoSeq, additionalThoughtInfo.me, additionalThoughtInfo.enemy, depth, iteration);

    const Plan& plan = thoughtResult.plan;
    if (plan.decisions().empty())
        return DropDecision(Decision(3, 0), thoughtResult.message);
    return DropDecision(plan.decisions().front(), thoughtResult.message);
}

ThoughtResult MayahAI::thinkPlan(int frameId, const CoreField& field, const KumipuyoSeq& kumipuyoSeq,
                                 const PlayerState& me, const PlayerState& enemy,
                                 int depth, int maxIteration, vector<Decision>* specifiedDecisions)
{
    double beginTime = currentTime();

    LOG(INFO) << "\n" << field.toDebugString() << "\n" << kumipuyoSeq.toString();
    if (VLOG_IS_ON(1)) {
        VLOG(1) << "\n"
                << "----------------------------------------------------------------------" << endl
                << "think frameId = " << frameId << endl
                << "my ojama: fixed = " << me.fixedOjama << " pending = " << me.pendingOjama << endl
                << "enemy ojama: fixed = " << enemy.fixedOjama << " pending = " << enemy.pendingOjama << endl
                << "enemy rensa: ending = " << (enemy.isRensaOngoing ? enemy.finishingRensaFrameId : 0) << endl
                << gazer_.gazeResult().toRensaInfoString()
                << "----------------------------------------------------------------------" << endl;
    }

    if (usesDecisionBook_) {
        Decision d = decisionBook_.nextDecision(field, kumipuyoSeq);
        if (d.isValid()) {
            CoreField cf(field);
            cf.dropKumipuyo(d, kumipuyoSeq.front());
            vector<Decision> decisions { d };

            ThoughtResult tr(Plan(cf, decisions, RensaResult(), 0, 0, 0),
                             false, 0.0, 0.0, MidEvalResult(), "BY DECISION BOOK");
            return tr;
        }
    }

    const GazeResult gazeResult = gazer_.gazeResult();

    // Before evaling, check Book.
    const PreEvalResult preEvalResult = preEval(field);

    Plan bestPlan;
    double bestScore = -100000000.0;
    MidEvalResult bestMidEvalResult;

    Plan bestRensaPlan;
    int bestRensaScore = 0;
    int bestRensaFrames = 0;
    MidEvalResult bestRensaMidEvalResult;

    int bestVirtualRensaScore = 0;

    mutex mu;
    auto evalRefPlan = [&, this, frameId, maxIteration](const RefPlan& plan, const MidEvalResult& midEvalResult) {
        if (specifiedDecisions && plan.decisions() != *specifiedDecisions)
            return;

        EvalResult evalResult = eval(plan, field, frameId, maxIteration, me, enemy, preEvalResult, midEvalResult, gazeResult);

        VLOG(1) << toString(plan.decisions())
                << ": eval=" << evalResult.score()
                << " pscore=" << plan.score()
                << " vscore=" << evalResult.maxVirtualScore();

        lock_guard<mutex> lock(mu);

        if (bestScore < evalResult.score()) {
            bestScore = evalResult.score();
            bestPlan = plan.toPlan();
            bestMidEvalResult = midEvalResult;
        }

        if (bestVirtualRensaScore < evalResult.maxVirtualScore()) {
            bestVirtualRensaScore = evalResult.maxVirtualScore();
        }

        if (bestRensaScore < plan.score() || (bestRensaScore == plan.score() && bestRensaFrames > plan.totalFrames())) {
            bestRensaScore = plan.score();
            bestRensaFrames = plan.totalFrames();
            bestRensaPlan = plan.toPlan();
            bestRensaMidEvalResult = midEvalResult;
        }
    };

    // NOTE: Since RefPlan will be destructed after this callback, if we'd like to pass RefPlan to
    // an executor, we need to make Plan, and copy it.

    WaitGroup wg;
    auto evalAfterOne = [&, this, depth](const RefPlan& rp1) {
        if (specifiedDecisions) {
            if (rp1.decisions().empty() || specifiedDecisions->empty())
                return;
            if (rp1.decisions()[0] != (*specifiedDecisions)[0])
                return;
        }

        // Do eval after one drop.
        if (rp1.isRensaPlan()) {
            if (executor_) {
                wg.add(1);
                Plan p = rp1.toPlan();
                executor_->submit([p, &wg, &evalRefPlan]() {
                    evalRefPlan(RefPlan(p), MidEvalResult());
                    wg.done();
                });
            } else {
                evalRefPlan(rp1, MidEvalResult());
            }
        }

        MidEvalResult midEvalResult = MidEvaluator().eval(rp1, field);

        Plan p = rp1.toPlan();
        KumipuyoSeq seq(kumipuyoSeq);
        if (seq.size() > 0)
            seq.dropFront();

        auto f = [p, midEvalResult, &evalRefPlan](const RefPlan& plan) {
            vector<Decision> decisions(p.decisions());
            decisions.insert(decisions.end(), plan.decisions().begin(), plan.decisions().end());
            RefPlan refPlan(plan.field(),
                            decisions,
                            plan.rensaResult(),
                            p.numChigiri() + plan.numChigiri(),
                            p.totalFrames() + plan.framesToInitiate(),
                            plan.lastDropFrames());
            evalRefPlan(refPlan, midEvalResult);
        };

        if (executor_) {
            wg.add(1);
            executor_->submit([p, seq, depth, f, &wg]() {
                Plan::iterateAvailablePlans(p.field(), seq, depth - 1, f);
                wg.done();
            });
        } else {
            Plan::iterateAvailablePlans(p.field(), seq, depth - 1, f);
        }
    };

    Plan::iterateAvailablePlans(field, kumipuyoSeq, 1, evalAfterOne);
    if (executor_)
        wg.waitUntilDone();

    double endTime = currentTime();
    if (bestVirtualRensaScore < bestRensaScore) {
        std::string message = makeMessageFrom(frameId, field, kumipuyoSeq, maxIteration,
                                              me, enemy,
                                              preEvalResult, bestRensaMidEvalResult, gazeResult,
                                              bestRensaPlan, bestRensaScore, bestVirtualRensaScore, endTime - beginTime);
        return ThoughtResult(bestRensaPlan, true, bestRensaScore, bestVirtualRensaScore, bestRensaMidEvalResult, message);
    } else {
        std::string message = makeMessageFrom(frameId, field, kumipuyoSeq, maxIteration,
                                              me, enemy,
                                              preEvalResult, bestMidEvalResult, gazeResult,
                                              bestPlan, bestRensaScore, bestVirtualRensaScore, endTime - beginTime);
        return ThoughtResult(bestPlan, false, bestRensaScore, bestVirtualRensaScore, bestMidEvalResult, message);
    }
}

PreEvalResult MayahAI::preEval(const CoreField& currentField)
{
    PreEvaluator preEvaluator(openingBook_);
    return preEvaluator.preEval(currentField);
}

EvalResult MayahAI::eval(const RefPlan& plan, const CoreField& currentField,
                         int currentFrameId, int maxIteration,
                         const PlayerState& me, const PlayerState& enemy,
                         const PreEvalResult& preEvalResult,
                         const MidEvalResult& midEvalResult,
                         const GazeResult& gazeResult) const
{
    NormalScoreCollector sc(*evaluationParameter_);
    Evaluator<NormalScoreCollector> evaluator(openingBook_, &sc);
    evaluator.collectScore(plan, currentField, currentFrameId, maxIteration, me, enemy, preEvalResult, midEvalResult, gazeResult);

    return EvalResult(sc.score(), sc.estimatedRensaScore());
}

CollectedFeature MayahAI::evalWithCollectingFeature(const RefPlan& plan, const CoreField& currentField,
                                                    int currentFrameId, int maxIteration,
                                                    const PlayerState& me,
                                                    const PlayerState& enemy,
                                                    const PreEvalResult& preEvalResult,
                                                    const MidEvalResult& midEvalResult,
                                                    const GazeResult& gazeResult) const
{
    FeatureScoreCollector sc(*evaluationParameter_);
    Evaluator<FeatureScoreCollector> evaluator(openingBook_, &sc);
    evaluator.collectScore(plan, currentField, currentFrameId, maxIteration, me, enemy, preEvalResult, midEvalResult, gazeResult);
    return sc.toCollectedFeature();
}

std::string MayahAI::makeMessageFrom(int frameId, const CoreField& field, const KumipuyoSeq& kumipuyoSeq, int maxIteration,
                                     const PlayerState& me, const PlayerState& enemy,
                                     const PreEvalResult& preEvalResult, const MidEvalResult& midEvalResult, const GazeResult& gazeResult,
                                     const Plan& plan, double rensaScore, double virtualRensaScore, double thoughtTimeInSeconds) const
{
    UNUSED_VARIABLE(kumipuyoSeq);

    if (plan.decisions().empty())
        return string("give up :-(");

    RefPlan refPlan(plan.field(), plan.decisions(), plan.rensaResult(), plan.numChigiri(), plan.framesToInitiate(), plan.lastDropFrames());
    CollectedFeature cf = evalWithCollectingFeature(refPlan, field, frameId, maxIteration, me, enemy, preEvalResult, midEvalResult, gazeResult);

    stringstream ss;
    if (cf.feature(STRATEGY_ZENKESHI) > 0 || cf.feature(STRATEGY_INITIAL_ZENKESHI) > 0)
        ss << "ZENKESHI / ";
    if (cf.feature(STRATEGY_TAIOU) > 0)
        ss << "TAIOU / ";
    if (cf.feature(STRATEGY_LARGE_ENOUGH) > 0)
        ss << "LARGE_ENOUGH / ";
    if (cf.feature(STRATEGY_TSUBUSHI) > 0)
        ss << "TSUBUSHI / ";
    if (cf.feature(STRATEGY_SAISOKU) > 0)
        ss << "SAISOKU / ";
    else if (cf.feature(STRATEGY_SAKIUCHI) > 0)
        ss << "SAKIUCHI / ";
    if (cf.feature(STRATEGY_ZENKESHI_CONSUME) > 0)
        ss << "USE_ZENKESHI / ";
    if (cf.feature(STRATEGY_LAND_LEVELING) > 0)
        ss << "LEVELING / ";

    if (!cf.bookName().empty())
        ss << cf.bookName() << " / ";

    ss << "D/I = " << plan.decisions().size() << "/" << maxIteration << " / ";
    ss << "SCORE = " << cf.score() << " / ";

    if (!cf.feature(MAX_CHAINS).empty()) {
        const vector<int>& vs = cf.feature(MAX_CHAINS);
        for (size_t i = 0; i < vs.size(); ++i)
            ss << "MAX CHAIN = " << vs[i] << " / ";
    }

    ss << "R/V SCORE=" << rensaScore << "/" << virtualRensaScore << " / ";

    if (enemy.isRensaOngoing) {
        ss << "Gazed ongoing rensa : " << enemy.ongoingRensaResult.score
           << " in " << (enemy.finishingRensaFrameId - frameId) << " / ";
    } else {
        ss << "Gazed = "
           << gazeResult.estimateMaxScore(frameId + refPlan.totalFrames(), enemy)
           << " in " << refPlan.totalFrames() << " / "
           << gazeResult.estimateMaxScore(frameId + refPlan.totalFrames() + 100, enemy)
           << " in " << (refPlan.totalFrames() + 100) << " / "
           << gazeResult.estimateMaxScore(frameId + refPlan.totalFrames() + 200, enemy)
           << " in " << (refPlan.totalFrames() + 200) << " / ";
    }

    ss << "O = " << (myPlayerState().fixedOjama + myPlayerState().pendingOjama)
       << "/" << (enemyPlayerState().fixedOjama + enemyPlayerState().pendingOjama) << " / ";

    ss << (thoughtTimeInSeconds * 1000) << " [ms]";

    return ss.str();
}

void MayahAI::onGameWillBegin(const FrameRequest& frameRequest)
{
    gazer_.initialize(frameRequest.frameId);
}

void MayahAI::gaze(int frameId, const CoreField& enemyField, const KumipuyoSeq& kumipuyoSeq)
{
    gazer_.gaze(frameId, enemyField, kumipuyoSeq);
}
