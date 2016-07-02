#include "evaluator.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>

#include <glog/logging.h>

#include "base/time.h"
#include "core/plan/plan.h"
#include "core/rensa/rensa_detector.h"
#include "core/core_field.h"
#include "core/decision.h"
#include "core/field_checker.h"
#include "core/position.h"
#include "core/probability/column_puyo_list_probability.h"
#include "core/probability/puyo_set_probability.h"
#include "core/rensa_result.h"
#include "core/score.h"

#include "evaluation_parameter.h"
#include "gazer.h"
#include "move_evaluator.h"
#include "pattern_rensa_detector.h"
#include "rensa_evaluator.h"
#include "rensa_hand_tree.h"
#include "shape_evaluator.h"

using namespace std;

// ----------------------------------------------------------------------

MidEvalResult MidEvaluator::eval(const RefPlan& plan, const CoreField& currentField, double score)
{
    UNUSED_VARIABLE(currentField);

    MidEvalResult result;
    if (plan.isRensaPlan())
        result.add(MIDEVAL_ERASE, 1);

    result.add(MIDEVAL_RESULT, score);
    return result;
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalFallenOjama(int fallenOjama)
{
    sc_->addScore(FALLEN_OJAMA, fallenOjama);
}

// Returns true If we don't need to evaluate other features.
template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalStrategy(const RefPlan& plan,
                                             int currentFrameId,
                                             int rensaTreeValue,
                                             const PlayerState& me,
                                             const PlayerState& enemy,
                                             const GazeResult& gazeResult,
                                             const MidEvalResult& midEvalResult)
{
    sc_->addScore(STRATEGY_RENSA_TREE, rensaTreeValue);
    if (rensaTreeValue < 0)
        sc_->addScore(STRATEGY_RENSA_TREE_NEGATIVE, rensaTreeValue);

    if (!plan.isRensaPlan())
        return;

#if 0
    if (plan.fallenOjama() > 0)
        return;
#endif

    int rensaEndingFrameId = currentFrameId + plan.totalFrames();
    int estimatedMaxScore = gazeResult.estimateMaxScore(rensaEndingFrameId, enemy);

    if (!enemy.isRensaOngoing() && plan.chains() <= 4) {
        int h = 12 - enemy.field.height(3);
        if (plan.score() >= estimatedMaxScore + scoreForOjama(6 * h + plan.totalOjama())) {
            sc_->addScore(STRATEGY_KILL, 1);
            sc_->addScore(STRATEGY_FRAMES, plan.totalFrames());
            return;
        }
    }

    // --- If the rensa is large enough, fire it.
    if (!enemy.isRensaOngoing()) {
        if (plan.score() >= estimatedMaxScore + scoreForOjama(60 + plan.totalOjama())) {
            sc_->addScore(STRATEGY_LARGE_ENOUGH, 1.0);
            sc_->addScore(STRATEGY_FRAMES, plan.totalFrames());
            return;
        }
    }

#if 0
    // TAIOU
    if (me.totalOjama(enemy) > 0 && rensaTreeValue >= 6) {
        if (plan.totalOjama() == 0) {
            sc_->addScore(STRATEGY_TAIOU, 1);
            sc_->addScore(STRATEGY_FRAMES, plan.totalFrames());
            return;
        } else if (plan.totalOjama() < 3) {
            sc_->addScore(STRATEGY_TAIOU, 0.9);
            sc_->addScore(STRATEGY_FRAMES, plan.totalFrames());
            return;
        }
    }

    if (me.totalOjama(enemy) >= 9 && rensaTreeValue < 0) {
        if (plan.totalOjama() == 0) {
            sc_->addScore(STRATEGY_TAIOU_RELUCTANT, 1);
            sc_->addScore(STRATEGY_FRAMES, plan.totalFrames());
            return;
        } else if (plan.totalOjama() < 3) {
            sc_->addScore(STRATEGY_TAIOU_RELUCTANT, 0.9);
            sc_->addScore(STRATEGY_FRAMES, plan.totalFrames());
            return;
        }
    }

#else
    if (me.totalOjama(enemy) >= 3) {
        if (plan.score() >= scoreForOjama(std::max(0, me.totalOjama(enemy) - 3))) {
            sc_->addScore(STRATEGY_TAIOU, 1.0);
            sc_->addScore(STRATEGY_FRAMES, plan.totalFrames());
            return;
        }
    }

#if 0
    if (plan.totalOjama() >= 6) {
        if (plan.score() >= scoreForOjama(std::max(0, plan.totalOjama() - 3))) {
            sc_->addScore(STRATEGY_TAIOU, 0.9);
            sc_->addScore(STRATEGY_FRAMES, plan.totalFrames());
            return;
        }
    }
#endif
#endif

    if (plan.field().isZenkeshi()) {
        sc_->addScore(STRATEGY_SCORE, plan.score());
        sc_->addScore(STRATEGY_ZENKESHI, 1);
        // Don't consider frames.
        return;
    }

    // not plan.hasZenekshi, since it's already consumed.
    if (me.hasZenkeshi && !enemy.hasZenkeshi) {
        if (!enemy.isRensaOngoing()) {
            sc_->addScore(STRATEGY_SCORE, plan.score());
            sc_->addScore(STRATEGY_SOLO_ZENKESHI_CONSUME, 1);
            sc_->addScore(STRATEGY_FRAMES, plan.totalFrames());
            return;
        }
        if (plan.pendingOjama() + plan.fixedOjama() <= 36) {
            sc_->addScore(STRATEGY_SCORE, plan.score());
            sc_->addScore(STRATEGY_SOLO_ZENKESHI_CONSUME, 1);
            sc_->addScore(STRATEGY_FRAMES, plan.totalFrames());
            return;
        }
    }

#if 1
    if (plan.chains() == 2 && plan.totalOjama() <= 3 && midEvalResult.feature(MIDEVAL_ERASE) == 0 && rensaTreeValue > 0) {
        if (plan.score() >= scoreForOjama(24)) {
            sc_->addScore(STRATEGY_FIRE_SIDE_CHAIN_2_LARGE, 1);
        } else if (plan.score() >= scoreForOjama(18)) {
            sc_->addScore(STRATEGY_FIRE_SIDE_CHAIN_2_MEDIUM, 1);
        } else if (plan.score() >= scoreForOjama(15)) {
            sc_->addScore(STRATEGY_FIRE_SIDE_CHAIN_2_SMALL, 1);
        }
        return ;
    }

    if (plan.chains() == 3 && plan.totalOjama() == 0 && midEvalResult.feature(MIDEVAL_ERASE) == 0 && rensaTreeValue > 0) {
        if (plan.score() >= scoreForOjama(30)) {
            sc_->addScore(STRATEGY_FIRE_SIDE_CHAIN_3_LARGE, 1);
        } else if (plan.score() >= scoreForOjama(18)) {
            sc_->addScore(STRATEGY_FIRE_SIDE_CHAIN_3_MEDIUM, 1);
        } else if (plan.score() >= scoreForOjama(15)) {
            sc_->addScore(STRATEGY_FIRE_SIDE_CHAIN_3_SMALL, 1);
        }
        return ;
    }
#endif

    // If IBARA found, we always consider it.
    // TODO(mayah): Don't consider IBARA if we don't have enough puyos. Better not to fire IBARA in that case.
    if (plan.chains() == 1 && plan.score() >= scoreForOjama(10) && rensaTreeValue >= 10) {
        sc_->addScore(STRATEGY_IBARA, 1);
        sc_->addScore(STRATEGY_FRAMES, plan.totalFrames());
        return;
    }

    // TSUBUSHI / SAISOKU
    if (!enemy.isRensaOngoing()) {
        if (plan.chains() <= 3 && plan.score() >= scoreForOjama(15) && rensaTreeValue >= 5) {
            sc_->addScore(STRATEGY_TSUBUSHI, 1);
            sc_->addScore(STRATEGY_FRAMES, plan.totalFrames());
            return;
        }

        if (plan.chains() == 2 && plan.score() >= scoreForOjama(15) && rensaTreeValue >= 5) {
            sc_->addScore(STRATEGY_TSUBUSHI, 1);
            sc_->addScore(STRATEGY_FRAMES, plan.totalFrames());
            return;
        }
    }

    if (me.hasZenkeshi && enemy.hasZenkeshi) {
        sc_->addScore(STRATEGY_ZENKESHI_CONSUME, 1);
        sc_->addScore(STRATEGY_FRAMES, plan.totalFrames());
        return;
    }

    sc_->addScore(STRATEGY_SAKIUCHI, 1.0);

    // TODO(mayah): Check land leveling.
    // TODO(mayah): OIUCHI?
    // TODO(mayah): KILL when enemy has a lot of puyos?
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::evalMidEval(const MidEvalResult& midEvalResult)
{
    // Copy midEvalResult.
    for (const auto& entry : midEvalResult.collectedFeatures()) {
        sc_->addScore(entry.first, entry.second);
    }
}

template<typename ScoreCollector>
CollectedCoef Evaluator<ScoreCollector>::calculateDefaultCoef(const PlayerState& me, const PlayerState& enemy) const
{
    const int INITIAL_THRESHOLD = 16;
    const int EARLY_THRESHOLD = 24;
    const int MIDDLE_THRESHOLD = 36;
    const int LATE_THRESHOLD = 54;

    const int count = me.field.countPuyos();

    CollectedCoef coef;

    if (enemy.hasZenkeshi) {
        if (count <= EARLY_THRESHOLD)
            coef.setCoef(EvaluationMode::ENEMY_HAS_ZENKESHI, 1.0);
        else
            coef.setCoef(EvaluationMode::ENEMY_HAS_ZENKESHI_MIDDLE, 1.0);
        return coef;
    }

    if (count <= INITIAL_THRESHOLD && me.hand <= 8) {
        coef.setCoef(EvaluationMode::INITIAL, 1.0);
        return coef;
    }

    if (count <= EARLY_THRESHOLD) {
        coef.setCoef(EvaluationMode::EARLY, 1.0);
        return coef;
    }

    if (count <= MIDDLE_THRESHOLD) {
        double ratio = static_cast<double>(count - EARLY_THRESHOLD) / (MIDDLE_THRESHOLD - EARLY_THRESHOLD);
        coef.setCoef(EvaluationMode::EARLY, 1 - ratio);
        coef.setCoef(EvaluationMode::MIDDLE, ratio);
        return coef;
    }

    if (count <= LATE_THRESHOLD) {
        double ratio = static_cast<double>(count - MIDDLE_THRESHOLD) / (LATE_THRESHOLD - MIDDLE_THRESHOLD);
        coef.setCoef(EvaluationMode::MIDDLE, 1 - ratio);
        coef.setCoef(EvaluationMode::LATE, ratio);
        return coef;
    }

    coef.setCoef(EvaluationMode::LATE, 1.0);
    return coef;
}

template<typename ScoreCollector>
void Evaluator<ScoreCollector>::eval(const RefPlan& plan,
                                     const KumipuyoSeq& restSeq,
                                     int currentFrameId,
                                     int maxIteration,
                                     const PlayerState& me,
                                     const PlayerState& enemy,
                                     const MidEvalResult& midEvalResult,
                                     bool fast,
                                     bool usesRensaHandTree,
                                     const GazeResult& gazeResult)
{
    typedef typename ScoreCollector::RensaScoreCollector RensaScoreCollector;
    typedef typename RensaScoreCollector::CollectedScore RensaCollectedScore;

    CollectedCoef coef = calculateDefaultCoef(me, enemy);
    sc_->setCoef(coef);

    const CoreField& fieldBeforeRensa = plan.field();

    MoveEvaluator<ScoreCollector>(sc_).eval(plan);
    ShapeEvaluator<ScoreCollector>(sc_).eval(plan.field());

    evalMidEval(midEvalResult);
    evalFallenOjama(plan.fallenOjama());

    const int numReachableSpace = fieldBeforeRensa.countConnectedPuyos(3, 12);
    int maxChain = 0;
    int rensaCounts[20] {};

    int sideChainMaxScore = 0;
    int fastChain4MaxScore = 0;
    int fastChain6MaxScore = 0;
    int fastChain10MaxScore = 0;
    int maxVirtualRensaResultScore = 0;

    // TODO(mayah): MainRensaInfo should be array for each mode?
    struct MainRensaInfo {
        double score = -100000000;
        RensaCollectedScore collectedScore;
        int maxChains = 0;
    } mainRensa;

    // TODO(mayah): fill this.
    struct SideRensaInfo {
        double score = -100000000;
        RensaCollectedScore collectedScore;
    } sideRensa;

    RensaHandNodeMaker handTreeMaker(2, restSeq);
    auto evalCallback = [&](const CoreField& fieldAfterRensa,
                            const RensaResult& rensaResult,
                            const ColumnPuyoList& puyosToComplement,
                            PuyoColor /*firePuyoColor*/,
                            const string& patternName,
                            double patternScore) {
        CoreField complementedField(fieldBeforeRensa);
        if (!complementedField.dropPuyoList(puyosToComplement))
            return;

        ++rensaCounts[rensaResult.chains];

        const PuyoSet necessaryPuyoSet(puyosToComplement);
        const PuyoSetProbability* psp = PuyoSetProbability::instanceSlow();
        const double possibility = psp->possibility(necessaryPuyoSet, std::max(0, numReachableSpace));
        const double virtualRensaScore = rensaResult.score * possibility;

        RensaScoreCollector rensaScoreCollector(sc_->mainRensaParamSet(), sc_->sideRensaParamSet());
        RensaEvaluator<RensaScoreCollector> rensaEvaluator(patternBook(), &rensaScoreCollector);
        rensaEvaluator.eval(complementedField, fieldBeforeRensa, fieldAfterRensa,
                            rensaResult, puyosToComplement, patternScore, virtualRensaScore);
        rensaEvaluator.evalRensaStrategy(plan, rensaResult, puyosToComplement, currentFrameId, me, enemy);

        // TODO(mayah): need to set a better mode here.
        if (mainRensa.score < rensaScoreCollector.mainRensaScore().score(coef)) {
            rensaScoreCollector.setBookname(patternName);
            rensaScoreCollector.setPuyosToComplement(puyosToComplement);
            mainRensa.score = rensaScoreCollector.mainRensaScore().score(coef);
            mainRensa.collectedScore = rensaScoreCollector.mainRensaScore();
        }

        if (sideRensa.score < rensaScoreCollector.sideRensaScore().score(coef)) {
            rensaScoreCollector.setPuyosToComplement(puyosToComplement);
            sideRensa.score = rensaScoreCollector.sideRensaScore().score(coef);
            sideRensa.collectedScore = rensaScoreCollector.sideRensaScore();
        }

        if (maxChain < rensaResult.chains) {
            maxChain = rensaResult.chains;
        }

        if (maxVirtualRensaResultScore < virtualRensaScore) {
            maxVirtualRensaResultScore = virtualRensaScore;
        }

        if (puyosToComplement.size() <= 1 && rensaResult.chains == 2) {
            sideChainMaxScore = std::max<int>(sideChainMaxScore, rensaResult.score);
        }
        if (puyosToComplement.size() <= 2 && rensaResult.chains == 2) {
            sideChainMaxScore = std::max<int>(sideChainMaxScore, rensaResult.score * 0.9);
        }
#if 0
        if (puyosToComplement.size() <= 3 && rensaResult.chains == 2) {
            sideChainMaxScore = std::max<int>(sideChainMaxScore, rensaResult.score * 0.8);
        }
        if (puyosToComplement.size() <= 4 && rensaResult.chains == 2) {
            sideChainMaxScore = std::max<int>(sideChainMaxScore, rensaResult.score * 0.75);
        }
#endif

        const ColumnPuyoListProbability* cplp = ColumnPuyoListProbability::instanceSlow();
        int necessaryKumipuyos = cplp->necessaryKumipuyos(puyosToComplement);
        if (necessaryKumipuyos <= 2 && fastChain4MaxScore < rensaResult.score) {
            fastChain4MaxScore = rensaResult.score;
        }
        if (necessaryKumipuyos <= 3 && fastChain6MaxScore < rensaResult.score) {
            fastChain6MaxScore = rensaResult.score;
        }
        if (necessaryKumipuyos <= 5 && fastChain10MaxScore < rensaResult.score) {
            fastChain10MaxScore = rensaResult.score;
        }

        // Now, we can simulate complementedField.
        if (!fast && usesRensaHandTree) {
            handTreeMaker.add(std::move(complementedField), puyosToComplement, 0, PuyoSet());
        }
    };

    PatternRensaDetector detector(patternBook(), fieldBeforeRensa, evalCallback);
    detector.iteratePossibleRensas(maxIteration);

    RensaDetector::detectSideChain(fieldBeforeRensa, RensaDetectorStrategy::defaultDropStrategy(),
                                   [&](CoreField&& cf, const ColumnPuyoList& cpl) {
        // TODO(mayah): fireColor is not PuyoColor::EMPTY.
        RensaResult rensaResult = cf.simulate();
        evalCallback(cf, rensaResult, cpl, PuyoColor::EMPTY, string(), 0.0);
    });

    int rensaHandValue = 0;
    if (!fast && usesRensaHandTree) {
        RensaHandTree myRensaTree(vector<RensaHandNode>{ handTreeMaker.makeNode() });
        // TODO(mayah): num ojama is correct? frame id is correct? not sure...
        int myOjama = plan.totalOjama();
        int myOjamaCommittingFrameId = plan.ojamaCommittingFrameId();
        int enemyOjama = 0;
        int enemyOjamaCommittingFrameId = 0;
        if (plan.isRensaPlan()) {
            int ojama = plan.rensaResult().score / 70;
            if (me.totalOjama(enemy) - ojama > 0) {
                myOjama = me.totalOjama(enemy) - ojama;
                myOjamaCommittingFrameId = plan.ojamaCommittingFrameId();
                enemyOjama = 0;
                enemyOjamaCommittingFrameId = 0;
            } else {
                int restOjama = ojama - me.totalOjama(enemy);
                myOjama = 0;
                myOjamaCommittingFrameId = 0;
                enemyOjama = enemy.totalOjama(me) + restOjama;
                if (enemyOjama > 0)
                    enemyOjamaCommittingFrameId = currentFrameId + plan.totalFrames();
            }
        } else {
            myOjama = plan.totalOjama();
            if (myOjama > 0)
                myOjamaCommittingFrameId = plan.ojamaCommittingFrameId();
            enemyOjama = enemy.totalOjama(me);
            if (enemyOjama > 0)
                enemyOjamaCommittingFrameId = currentFrameId;
        }

        double feasibleBeginTime = currentTime();
        int feasibleRensaHandValue =
            RensaHandTree::eval(myRensaTree, currentFrameId + plan.totalFrames(), 0, myOjama, myOjamaCommittingFrameId,
                                gazeResult.feasibleRensaHandTree(), gazeResult.frameIdToStartNextMove(), 0,
                                enemyOjama, enemyOjamaCommittingFrameId);
        double feasibleEndTime = currentTime();

        int possibleBeginTime = currentTime();
        int possibleRensaHandValue =
            RensaHandTree::eval(myRensaTree, currentFrameId + plan.totalFrames(), 0, myOjama, myOjamaCommittingFrameId,
                                gazeResult.possibleRensaHandTree(), gazeResult.frameIdToStartNextMove(), 0, enemyOjama, enemyOjamaCommittingFrameId);
        int possibleEndTime = currentTime();

        VLOG_IF(1, !sc_->isSimple())
            << "######################################################################\n"
            << fieldBeforeRensa.toDebugString()
            << restSeq.toString();

        VLOG(1) << "RensaHandTree::eval"
                << " feasible_score=" << feasibleRensaHandValue
                << " possible_score=" << possibleRensaHandValue
                << " myFrameId=" << currentFrameId + plan.totalFrames()
                << " myOjama=" << plan.totalOjama()
                << " myOjamaCommittingFrameId=" << myOjamaCommittingFrameId
                << " enemyFrameId=" << gazeResult.frameIdToStartNextMove()
                << " enemyOjama=" << enemyOjama
                << " enemyOjamaCommittingFrameId=" << enemyOjamaCommittingFrameId
                << " feasible_time=" << (1000 * (feasibleEndTime - feasibleBeginTime))
                << " possible_time=" << (1000 * (possibleEndTime - possibleBeginTime));

        VLOG_IF(1, !sc_->isSimple()) << "RensaHandTree trees:\n"
            << " feasible_score=" << feasibleRensaHandValue << "\n"
            << " possible_score=" << possibleRensaHandValue << "\n"
            << "MyTree:\n"
            << myRensaTree.toString()
            << "----------------------------------------------------------------------\n"
            << "Feasible EnemyTree:\n"
            << gazeResult.feasibleRensaHandTree().toString()
            << "----------------------------------------------------------------------\n"
            << "Possible EnemyTree:\n"
            << gazeResult.possibleRensaHandTree().toString()
            << "----------------------------------------------------------------------\n";

        rensaHandValue = std::min(feasibleRensaHandValue, possibleRensaHandValue);
    }

    evalStrategy(plan, currentFrameId, rensaHandValue, me, enemy, gazeResult, midEvalResult);

    // max chain
    sc_->addScore(RENSA_KIND, rensaCounts[maxChain]);

#if 0
    // side chain
    if (sideChainMaxScore >= scoreForOjama(21)) {
        sc_->addScore(HOLDING_SIDE_CHAIN_LARGE, 1);
    } else if (sideChainMaxScore >= scoreForOjama(15)) {
        sc_->addScore(HOLDING_SIDE_CHAIN_MEDIUM, 1);
    } else if (sideChainMaxScore >= scoreForOjama(12)) {
        sc_->addScore(HOLDING_SIDE_CHAIN_SMALL, 1);
    }
#endif

#if 1
    // fast chain
    if (fastChain6MaxScore >= gazeResult.estimateMaxScore(currentFrameId + NUM_FRAMES_OF_ONE_HAND * 3, enemy)) {
        sc_->addScore(KEEP_FAST_LARGER_THEN_ENEMY, 1);
    }
    if (fastChain4MaxScore >= scoreForOjama(18)) {
        sc_->addScore(KEEP_FAST_4_CHAIN, 1);
    }
    if (fastChain6MaxScore >= scoreForOjama(18)) {
        sc_->addScore(KEEP_FAST_6_CHAIN, 1);
    }
    if (fastChain10MaxScore >= scoreForOjama(30)) {
        sc_->addScore(KEEP_FAST_10_CHAIN, 1);
    }
#endif
#if 0
   if (fastChain4MaxScore >= gazeResult.estimateMaxScore(currentFrameId + NUM_FRAMES_OF_ONE_HAND * 2, enemy)) {
        sc_->addScore(KEEP_FAST_4_CHAIN, 1);
    }
    if (fastChain6MaxScore >= gazeResult.estimateMaxScore(currentFrameId + NUM_FRAMES_OF_ONE_HAND * 3, enemy)) {
        sc_->addScore(KEEP_FAST_6_CHAIN, 1);
    }
    if (fastChain10MaxScore >= gazeResult.estimateMaxScore(currentFrameId + NUM_FRAMES_OF_ONE_HAND * 5, enemy)) {
        sc_->addScore(KEEP_FAST_10_CHAIN, 1);
    }
#endif

    // finalize.
    sc_->mergeMainRensaScore(mainRensa.collectedScore);
    sc_->mergeSideRensaScore(sideRensa.collectedScore);
    sc_->setEstimatedRensaScore(maxVirtualRensaResultScore);
}

template class Evaluator<FeatureScoreCollector>;
template class Evaluator<SimpleScoreCollector>;
