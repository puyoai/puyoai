#include "mayah_ai.h"

#include <iostream>
#include <sstream>

#include <gflags/gflags.h>

#include "base/time.h"
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
DEFINE_bool(log_max_score, false, "log max score to stderr");

using namespace std;

MayahAI::MayahAI(int argc, char* argv[]) :
    AI(argc, argv, "mayah")
{
    setBehaviorRethinkAfterOpponentRensa(true);

    featureParameter_.reset(new FeatureParameter(FLAGS_feature));
    books_ = BookReader::parse(FLAGS_book);
    LOG(INFO) << featureParameter_->toString();
    for (const auto& bf : books_) {
        VLOG(1) << bf.toDebugString();
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
    double beginTime = currentTime();
    Plan plan = thinkPlan(frameId, f, kumipuyoSeq, additionalInfo,
                          MayahAI::DEFAULT_DEPTH, MayahAI::DEFAULT_NUM_ITERATION);
    double endTime = currentTime();
    std::string message = makeMessageFrom(frameId, f, kumipuyoSeq, MayahAI::DEFAULT_NUM_ITERATION, plan, endTime - beginTime);
    if (plan.decisions().empty())
        return DropDecision(Decision(3, 0), message);
    return DropDecision(plan.decisions().front(), message);
}

DropDecision MayahAI::thinkFast(int frameId, const PlainField& plainField, const KumipuyoSeq& kumipuyoSeq,
                                const AdditionalThoughtInfo& additionalInfo)
{
    CoreField f(plainField);
    double beginTime = currentTime();
    Plan plan = thinkPlan(frameId, f, kumipuyoSeq, additionalInfo,
                          MayahAI::DEFAULT_DEPTH, MayahAI::FAST_NUM_ITERATION);
    double endTime = currentTime();
    std::string message = makeMessageFrom(frameId, f, kumipuyoSeq, MayahAI::FAST_NUM_ITERATION, plan, endTime - beginTime);
    if (plan.decisions().empty())
        return DropDecision(Decision(3, 0), message);
    return DropDecision(plan.decisions().front(), message);
}

Plan MayahAI::thinkPlan(int frameId, const CoreField& field, const KumipuyoSeq& kumipuyoSeq,
                        const AdditionalThoughtInfo& additionalInfo,
                        int depth, int maxIteration)
{
    LOG(INFO) << "\n" << field.toDebugString() << "\n" << kumipuyoSeq.toString();
    VLOG(1) << gazer_.toRensaInfoString();

    gazer_.setAdditionalThoughtInfo(additionalInfo);

    int bestRensaScore = 0;
    int bestRensaFrames = 0;
    int bestVirtualRensaScore = 0;
    Plan bestRensaPlan;

    double bestScore = -100000000.0;
    Plan bestPlan;
    auto f = [&, this, frameId, maxIteration](const RefPlan& plan) {
        EvalResult evalResult = eval(plan, field, frameId, maxIteration);

        VLOG(1) << toString(plan.decisions())
                << ": eval=" << evalResult.score()
                << " pscore=" << plan.score()
                << " vscore=" << evalResult.maxVirtualScore();

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

        thoughtMaxScore_ = max(thoughtMaxScore_, plan.score());
        thoughtMaxRensa_ = max(thoughtMaxRensa_, plan.chains());
    };

    Plan::iterateAvailablePlans(field, kumipuyoSeq, depth, f);
    if (bestVirtualRensaScore < bestRensaScore) {
        return bestRensaPlan;
    }
    return bestPlan;
}

EvalResult MayahAI::eval(const RefPlan& plan, const CoreField& currentField,
                           int currentFrameId, int maxIteration) const
{
    NormalScoreCollector sc(*featureParameter_);
    Evaluator<NormalScoreCollector> evaluator(books_, &sc);
    evaluator.collectScore(plan, currentField, currentFrameId, maxIteration, gazer_);

    return EvalResult(sc.score(), sc.estimatedRensaScore());
}

CollectedFeature MayahAI::evalWithCollectingFeature(const RefPlan& plan, const CoreField& currentField,
                                                    int currentFrameId, int maxIteration) const
{
    FeatureScoreCollector sc(*featureParameter_);
    Evaluator<FeatureScoreCollector> evaluator(books_, &sc);
    evaluator.collectScore(plan, currentField, currentFrameId, maxIteration, gazer_);
    return sc.toCollectedFeature();
}

std::string MayahAI::makeMessageFrom(int frameId, const CoreField& field, const KumipuyoSeq& kumipuyoSeq, int maxIteration,
                                     const Plan& plan, double thoughtTimeInSeconds) const
{
    UNUSED_VARIABLE(kumipuyoSeq);

    if (plan.decisions().empty())
        return string("give up :-(");

    RefPlan refPlan(plan.field(), plan.decisions(), plan.rensaResult(), plan.numChigiri(), plan.framesToInitiate(), plan.lastDropFrames());
    CollectedFeature cf = evalWithCollectingFeature(refPlan, field, frameId, maxIteration);

    stringstream ss;
    if (cf.feature(STRATEGY_ZENKESHI) > 0)
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
    if (cf.feature(STRATEGY_HOUWA) > 0)
        ss << "HOUWA / ";
    if (cf.feature(STRATEGY_ZENKESHI_CONSUME) > 0)
        ss << "全消消費 / ";

    if (!cf.bookName().empty())
        ss << cf.bookName() << " / ";

    ss << "SCORE = " << cf.score() << " / ";

    if (!cf.feature(MAX_CHAINS).empty()) {
        const vector<int>& vs = cf.feature(MAX_CHAINS);
        for (size_t i = 0; i < vs.size(); ++i)
            ss << "MAX CHAIN = " << vs[i] << " / ";
    }

    if (gazer_.isRensaOngoing()) {
        ss << "Gazed ongoing rensa : " << gazer_.ongoingRensaResult().score
           << " in " << (gazer_.ongoingRensaFinishingFrameId() - frameId) << " / ";
    } else {
        ss << "Gazed max score = "
           << gazer_.estimateMaxScore(frameId + refPlan.totalFrames())
           << " in " << refPlan.totalFrames() << " / "
           << gazer_.estimateMaxScore(frameId + refPlan.totalFrames() + 100)
           << " in " << (refPlan.totalFrames() + 100) << " / "
           << gazer_.estimateMaxScore(frameId + refPlan.totalFrames() + 200)
           << " in " << (refPlan.totalFrames() + 200) << " / ";
    }

    ss << (thoughtTimeInSeconds * 1000) << " [ms]";

    return ss.str();
}

void MayahAI::onGameWillBegin(const FrameRequest& frameRequest)
{
    thoughtMaxRensa_ = 0;
    thoughtMaxScore_ = 0;
    enemyField_.clear();
    gazer_.initialize(frameRequest.frameId);
}

void MayahAI::onGameHasEnded(const FrameRequest&)
{
    if (FLAGS_log_max_score) {
        cerr << "max rensa = " << thoughtMaxRensa_ << endl;
        cerr << "max score = " << thoughtMaxScore_ << endl;
    }
}

void MayahAI::onEnemyDecisionRequested(const FrameRequest& frameRequest)
{
    enemyField_ = CoreField(frameRequest.enemyPlayerFrameRequest().field);
    enemyField_.forceDrop();
    enemyDecisonRequestFrameId_ = frameRequest.frameId;
}

void MayahAI::onEnemyNext2Appeared(const FrameRequest& frameRequest)
{
    // At the beginning of the game, kumipuyoSeq might contain EMPTY/EMPTY.
    // In that case, we need to skip.
    const KumipuyoSeq& seq = frameRequest.enemyPlayerFrameRequest().kumipuyoSeq;
    if (!isNormalColor(seq.axis(0)) || !isNormalColor(seq.child(0)))
        return;

    // Since enemy's current moving puyo might be grounded already. In that case, we use the same puyo twice.
    // So, we need to use the field that is taken in DecisionRequest.
    gazer_.gaze(enemyDecisonRequestFrameId_, enemyField_, frameRequest.enemyPlayerFrameRequest().kumipuyoSeq);

    LOG(INFO) << '\n' << gazer_.toRensaInfoString();
}
