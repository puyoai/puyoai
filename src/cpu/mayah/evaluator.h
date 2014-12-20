#ifndef EVALUATION_FEATURE_COLLECTOR_H_
#define EVALUATION_FEATURE_COLLECTOR_H_

#include <map>
#include <vector>

#include "opening_book.h"
#include "evaluation_feature.h"
#include "score_collector.h"

class ColumnPuyoList;
class CoreField;
class GazeResult;
struct PlayerState;
class RefPlan;
struct RensaResult;
class RensaTrackResult;

const int EARLY_THRESHOLD = 24;
const int MIDDLE_THRESHOLD = 54;

class PreEvalResult {
public:
    explicit PreEvalResult(const std::vector<int>& matchableBookIds) :
        matchableBookIds_(matchableBookIds) {}

    const std::vector<int>& matchableBookIds() const { return matchableBookIds_; }

private:
    std::vector<int> matchableBookIds_;
};

class PreEvaluator {
public:
    explicit PreEvaluator(const OpeningBook& openingBook) : openingBook_(openingBook) {}

    PreEvalResult preEval(const CoreField& currentField);

private:
    const OpeningBook& openingBook_;
};

class MidEvalResult {
public:
    void add(EvaluationFeatureKey key, double value)
    {
        collectedFeatures_[key] = value;
    }

    double feature(EvaluationFeatureKey key) const {
        auto it = collectedFeatures_.find(key);
        if (it == collectedFeatures_.end())
            return 0;

        return it->second;
    }

    const std::map<EvaluationFeatureKey, double>& collectedFeatures() const { return collectedFeatures_; }

private:
    std::map<EvaluationFeatureKey, double> collectedFeatures_;
};

class MidEvaluator {
public:
    MidEvalResult eval(const RefPlan&, const CoreField& currentField);
};

class EvalResult {
public:
    constexpr EvalResult(double score, int maxVirtualScore) : score_(score), maxVirtualScore_(maxVirtualScore) {}

    double score() const { return score_; }
    int maxVirtualScore() const { return maxVirtualScore_; }

private:
    double score_;
    int maxVirtualScore_;
};

template<typename ScoreCollector>
class RensaEvaluator {
public:
    // Don't take ownership of |sc|.
    explicit RensaEvaluator(const OpeningBook& openingBook, ScoreCollector* sc) :
        openingBook_(openingBook),
        sc_(sc) {}

    void evalRensaChainFeature(const RefPlan&,
                               const RensaResult&,
                               const ColumnPuyoList& keyPuyos,
                               const ColumnPuyoList& firePuyos);
    void collectScoreForRensaGarbage(const CoreField& fieldAfterDrop);
    void evalRensaHandWidthFeature(const CoreField&, const RensaTrackResult&);
    void evalFirePointTabooFeature(const RefPlan&, const RensaTrackResult&);
    void evalRensaIgnitionHeightFeature(const RefPlan&, const RensaTrackResult&, bool enemyHasZenkeshi);
    void evalRensaConnectionFeature(const CoreField& fieldAfterDrop);
    void evalRensaStrategy(const RefPlan&, const RensaResult&, const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos,
                           int currentFrameId, const PlayerState& me, const PlayerState& enemy);
private:
    const OpeningBook& openingBook_;
    ScoreCollector* sc_;
};

template<typename ScoreCollector>
class Evaluator {
public:
    // Don't take ownership of |sc|.
    explicit Evaluator(const OpeningBook& openingBook, ScoreCollector* sc) :
        openingBook_(openingBook),
        sc_(sc) {}

    void collectScore(const RefPlan&, const CoreField& currentField, int currentFrameId, int maxIteration,
                      const PlayerState& me, const PlayerState& enemy,
                      const PreEvalResult&, const MidEvalResult&, const GazeResult&);

    // ----------------------------------------------------------------------

    bool evalStrategy(const RefPlan&, const CoreField& currentField, int currentFrameId,
                      const PlayerState& me, const PlayerState& enemy, const GazeResult&);

    // Returns true if complete match.
    bool evalBook(const OpeningBook&, const std::vector<int>& matchableBookIds, const RefPlan&, const MidEvalResult&);
    void evalFrameFeature(const RefPlan&);
    void evalRestrictedConnectionHorizontalFeature(const CoreField&);
    void evalThirdColumnHeightFeature(const RefPlan&);
    void evalValleyDepth(const CoreField&);
    void evalRidgeHeight(const CoreField&);
    void evalFieldUShape(const RefPlan&, bool enemyHasZenkeshi);
    void evalUnreachableSpace(const CoreField&);

    void evalMidEval(const MidEvalResult&);

    void collectScoreForConnection(const CoreField&);
    void evalCountPuyoFeature(const RefPlan& plan);

private:
    const OpeningBook& openingBook_;
    ScoreCollector* sc_;
};

#endif
