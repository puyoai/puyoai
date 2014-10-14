#include "mayah_ai.h"

#include <iostream>
#include <sstream>

#include <gflags/gflags.h>

#include "base/time.h"
#include "base/wait_group.h"
#include "core/algorithm/plan.h"
#include "core/algorithm/puyo_possibility.h"
#include "core/frame_request.h"

#include "book_field.h"
#include "book_reader.h"
#include "evaluator.h"
#include "feature_parameter.h"
#include "gazer.h"

DEFINE_string(feature, SRC_DIR "/cpu/mayah/feature.txt", "the path to feature parameter");
DEFINE_string(book, SRC_DIR "/cpu/mayah/book.txt", "the path to book");
DEFINE_bool(use_advanced_next, false, "Use enemy's NEXT sequence also");

using namespace std;

MayahAI::MayahAI(int argc, char* argv[], Executor* executor) :
    AI(argc, argv, "mayah"),
    executor_(executor)
{
    setBehaviorRethinkAfterOpponentRensa(true);

    featureParameter_.reset(new FeatureParameter(FLAGS_feature));
    books_ = BookReader::parse(FLAGS_book);

    VLOG(1) << featureParameter_->toString();
    if (VLOG_IS_ON(1)) {
      for (const auto& bf : books_) {
        VLOG(1) << bf.toDebugString();
      }
    }

    google::FlushLogFiles(google::INFO);
}

MayahAI::~MayahAI()
{
}

void MayahAI::reloadParameter()
{
    featureParameter_.reset(new FeatureParameter(FLAGS_feature));
}

DropDecision MayahAI::think(int frameId, const PlainField& plainField, const KumipuyoSeq& kumipuyoSeq,
                            const AdditionalThoughtInfo& additionalInfo)
{
    CoreField f(plainField);

    int depth;
    int iteration;
    if (FLAGS_use_advanced_next) {
        if (kumipuyoSeq.size() >= 3) {
            depth = 3;
            iteration = 2;
        } else if (f.countPuyos() <= 54) {
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

    ThoughtResult thoughtResult = thinkPlan(frameId, f, kumipuyoSeq, additionalInfo, depth, iteration);

    const Plan& plan = thoughtResult.plan;
    if (plan.decisions().empty())
        return DropDecision(Decision(3, 0), thoughtResult.message);
    return DropDecision(plan.decisions().front(), thoughtResult.message);
}

DropDecision MayahAI::thinkFast(int frameId, const PlainField& plainField, const KumipuyoSeq& kumipuyoSeq,
                                const AdditionalThoughtInfo& additionalInfo)
{
    CoreField f(plainField);
    ThoughtResult thoughtResult = thinkPlan(frameId, f, kumipuyoSeq, additionalInfo,
                                            MayahAI::DEFAULT_DEPTH, MayahAI::FAST_NUM_ITERATION);

    const Plan& plan = thoughtResult.plan;
    if (thoughtResult.plan.decisions().empty())
        return DropDecision(Decision(3, 0), thoughtResult.message);
    return DropDecision(plan.decisions().front(), thoughtResult.message);
}

ThoughtResult MayahAI::thinkPlan(int frameId, const CoreField& field, const KumipuyoSeq& kumipuyoSeq,
                                 const AdditionalThoughtInfo& additionalInfo,
                                 int depth, int maxIteration)
{
    double beginTime = currentTime();

    LOG(INFO) << "\n" << field.toDebugString() << "\n" << kumipuyoSeq.toString();
    VLOG(1) << gazer_.gazeResult().toRensaInfoString();

    gazer_.setAdditionalThoughtInfo(additionalInfo);
    const GazeResult gazeResult = gazer_.gazeResult();

    // Before evaling, check Book.
    const PreEvalResult preEvalResult = preEval(field);

    Plan bestPlan;
    double bestScore = -100000000.0;

    Plan bestRensaPlan;
    int bestRensaScore = 0;
    int bestRensaFrames = 0;

    int bestVirtualRensaScore = 0;

    mutex mu;
    auto evalRefPlan = [&, this, frameId, maxIteration](const RefPlan& plan) {
        EvalResult evalResult = eval(plan, field, frameId, maxIteration, preEvalResult, gazeResult);

        VLOG(1) << toString(plan.decisions())
                << ": eval=" << evalResult.score()
                << " pscore=" << plan.score()
                << " vscore=" << evalResult.maxVirtualScore();

        lock_guard<mutex> lock(mu);

        if (bestScore < evalResult.score()) {
            bestScore = evalResult.score();
            bestPlan = plan.toPlan();
        }

        if (bestVirtualRensaScore < evalResult.maxVirtualScore()) {
            bestVirtualRensaScore = evalResult.maxVirtualScore();
        }

        if (bestRensaScore < plan.score() || (bestRensaScore == plan.score() && bestRensaFrames > plan.totalFrames())) {
            bestRensaScore = plan.score();
            bestRensaFrames = plan.totalFrames();
            bestRensaPlan = plan.toPlan();
        }
    };

    // NOTE: Since RefPlan will be destructed after this callback, if we'd like to pass RefPlan to
    // an executor, we need to make Plan, and copy it.

    WaitGroup wg;
    auto evalAfterOne = [&, this, depth](const RefPlan& rp1) {
        // Do eval after one drop.
        if (rp1.isRensaPlan()) {
            if (executor_) {
                wg.add(1);
                Plan p = rp1.toPlan();
                executor_->submit([p, &wg, &evalRefPlan]() {
                    RefPlan refPlan(p.field(), p.decisions(),
                                    p.rensaResult(),
                                    p.numChigiri(),
                                    p.framesToInitiate(),
                                    p.lastDropFrames());
                    evalRefPlan(refPlan);
                    wg.done();
                });
            } else {
                evalRefPlan(rp1);
            }
        }

        Plan p = rp1.toPlan();
        KumipuyoSeq seq(kumipuyoSeq);
        if (seq.size() > 0)
            seq.dropFront();

        auto f = [p, &evalRefPlan](const RefPlan& plan) {
            vector<Decision> decisions(p.decisions());
            decisions.insert(decisions.end(), plan.decisions().begin(), plan.decisions().end());
            RefPlan refPlan(plan.field(),
                            decisions,
                            plan.rensaResult(),
                            p.numChigiri() + plan.numChigiri(),
                            p.totalFrames() + plan.framesToInitiate(),
                            plan.lastDropFrames());
            evalRefPlan(refPlan);
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
        std::string message = makeMessageFrom(frameId, field, kumipuyoSeq, maxIteration, preEvalResult, gazeResult,
                                              bestRensaPlan, bestRensaScore, bestVirtualRensaScore, endTime - beginTime);
        return ThoughtResult(bestRensaPlan, true, bestRensaScore, bestVirtualRensaScore, message);
    } else {
        std::string message = makeMessageFrom(frameId, field, kumipuyoSeq, maxIteration, preEvalResult, gazeResult,
                                              bestPlan, bestRensaScore, bestVirtualRensaScore, endTime - beginTime);
        return ThoughtResult(bestPlan, false, bestRensaScore, bestVirtualRensaScore, message);
    }
}

PreEvalResult MayahAI::preEval(const CoreField& currentField)
{
    PreEvaluator preEvaluator(books_);
    return preEvaluator.preEval(currentField);
}

EvalResult MayahAI::eval(const RefPlan& plan, const CoreField& currentField,
                         int currentFrameId, int maxIteration,
                         const PreEvalResult& preEvalResult, const GazeResult& gazeResult) const
{
    NormalScoreCollector sc(*featureParameter_);
    Evaluator<NormalScoreCollector> evaluator(books_, &sc);
    evaluator.collectScore(plan, currentField, currentFrameId, maxIteration, preEvalResult, gazeResult);

    return EvalResult(sc.score(), sc.estimatedRensaScore());
}

CollectedFeature MayahAI::evalWithCollectingFeature(const RefPlan& plan, const CoreField& currentField,
                                                    int currentFrameId, int maxIteration,
                                                    const PreEvalResult& preEvalResult, const GazeResult& gazeResult) const
{
    FeatureScoreCollector sc(*featureParameter_);
    Evaluator<FeatureScoreCollector> evaluator(books_, &sc);
    evaluator.collectScore(plan, currentField, currentFrameId, maxIteration, preEvalResult, gazeResult);
    return sc.toCollectedFeature();
}

std::string MayahAI::makeMessageFrom(int frameId, const CoreField& field, const KumipuyoSeq& kumipuyoSeq, int maxIteration,
                                     const PreEvalResult& preEvalResult, const GazeResult& gazeResult,
                                     const Plan& plan, double rensaScore, double virtualRensaScore, double thoughtTimeInSeconds) const
{
    UNUSED_VARIABLE(kumipuyoSeq);

    if (plan.decisions().empty())
        return string("give up :-(");

    RefPlan refPlan(plan.field(), plan.decisions(), plan.rensaResult(), plan.numChigiri(), plan.framesToInitiate(), plan.lastDropFrames());
    CollectedFeature cf = evalWithCollectingFeature(refPlan, field, frameId, maxIteration, preEvalResult, gazeResult);

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

    ss << "RSCORE=" << rensaScore
       << "VSCORE=" << virtualRensaScore << " / ";

    if (gazeResult.isRensaOngoing()) {
        ss << "Gazed ongoing rensa : " << gazeResult.ongoingRensaResult().score
           << " in " << (gazeResult.ongoingRensaFinishingFrameId() - frameId) << " / ";
    } else {
        ss << "Gazed max score = "
           << gazeResult.estimateMaxScore(frameId + refPlan.totalFrames())
           << " in " << refPlan.totalFrames() << " / "
           << gazeResult.estimateMaxScore(frameId + refPlan.totalFrames() + 100)
           << " in " << (refPlan.totalFrames() + 100) << " / "
           << gazeResult.estimateMaxScore(frameId + refPlan.totalFrames() + 200)
           << " in " << (refPlan.totalFrames() + 200) << " / ";
    }

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
