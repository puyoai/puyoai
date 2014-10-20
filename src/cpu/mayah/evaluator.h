#ifndef EVALUATION_FEATURE_COLLECTOR_H_
#define EVALUATION_FEATURE_COLLECTOR_H_

#include <map>
#include <vector>

#include "book_field.h"
#include "feature_parameter.h"
#include "score_collector.h"

class ColumnPuyoList;
class CoreField;
class GazeResult;
class RefPlan;
struct RensaResult;
class RensaTrackResult;

class PreEvalResult {
public:
    explicit PreEvalResult(const std::vector<bool>& booksMatchable) :
        booksMatchable_(booksMatchable) {}

    const std::vector<bool>& booksMatchable() const { return booksMatchable_; }
private:
    std::vector<bool> booksMatchable_;
};

class PreEvaluator {
public:
    explicit PreEvaluator(const std::vector<BookField>& books) : books_(books) {}

    PreEvalResult preEval(const CoreField& currentField);

private:
    const std::vector<BookField>& books_;
};

class MidEvalResult {
public:
    void add(EvaluationFeatureKey key, double value)
    {
        collectedFeatures_[key] = value;
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
    explicit RensaEvaluator(const std::vector<BookField>& books, ScoreCollector* sc) :
        books_(books),
        sc_(sc) {}

    void evalRensaChainFeature(const RefPlan&,
                               const RensaResult&,
                               const ColumnPuyoList& keyPuyos,
                               const ColumnPuyoList& firePuyos);
    void collectScoreForRensaGarbage(const CoreField& fieldAfterDrop);
    void evalRensaHandWidthFeature(const RefPlan&, const RensaTrackResult&);
    void evalFirePointTabooFeature(const RefPlan&, const RensaTrackResult&);
    void evalRensaIgnitionHeightFeature(const RefPlan&, const RensaTrackResult&, bool enemyHasZenkeshi);
    void evalRensaConnectionFeature(const CoreField& fieldAfterDrop);
    void evalRensaStrategy(const RefPlan&, const RensaResult&, const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos,
                           int currentFrameId, const GazeResult&);
private:
    const std::vector<BookField>& books_;
    ScoreCollector* sc_;
};

template<typename ScoreCollector>
class Evaluator {
public:
    // Don't take ownership of |sc|.
    explicit Evaluator(const std::vector<BookField>& books, ScoreCollector* sc) :
        books_(books),
        sc_(sc) {}

    void collectScore(const RefPlan&, const CoreField& currentField, int currentFrameId, int maxIteration,
                      const PreEvalResult&, const MidEvalResult&, const GazeResult&);

    // ----------------------------------------------------------------------

    bool evalStrategy(const RefPlan& plan, const CoreField& currentField,
                      int currentFrameId, const GazeResult&);

    void evalBook(const std::vector<BookField>&, const std::vector<bool>& bookMatchable, const RefPlan&);
    void evalFrameFeature(const RefPlan&);
    void evalConnectionHorizontalFeature(const RefPlan&);
    void evalRestrictedConnectionHorizontalFeature(const RefPlan&);
    void evalDensityFeature(const RefPlan&);
    void evalFieldHeightFeature(const RefPlan&);
    void evalThirdColumnHeightFeature(const RefPlan& plan);
    void evalValleyDepth(const RefPlan& plan);
    void evalRidgeHeight(const RefPlan& plan);
    void evalFieldUShape(const RefPlan& plan, bool enemyHasZenkeshi);
    void evalUnreachableSpace(const RefPlan& plan);

    void evalMidEval(const MidEvalResult&);

    void collectScoreForConnection(const CoreField&);
    void evalCountPuyoFeature(const RefPlan& plan);

private:
    const std::vector<BookField>& books_;
    ScoreCollector* sc_;
};

#endif
