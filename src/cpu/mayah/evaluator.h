#ifndef CPU_MAYAH_EVALUATOR_H_
#define CPU_MAYAH_EVALUATOR_H_

#include <map>
#include <vector>

#include "core/pattern/pattern_book.h"

#include "evaluation_feature.h"
#include "score_collector.h"

class ColumnPuyoList;
class CoreField;
class GazeResult;
class KumipuyoSeq;
class RefPlan;

struct PlayerState;
struct RensaResult;

class EvaluatorBase {
protected:
    explicit EvaluatorBase(const PatternBook& patternBook) :
        patternBook_(patternBook) {}

    const PatternBook& patternBook() const { return patternBook_; }

private:
    const PatternBook& patternBook_;
};

class PreEvalResult {
public:
    PreEvalResult() {}
private:
};

class PreEvaluator : public EvaluatorBase {
public:
    explicit PreEvaluator(const PatternBook& patternBook) :
        EvaluatorBase(patternBook) {}

    PreEvalResult preEval(const CoreField& currentField);
};

class MidEvalResult {
public:
    void add(EvaluationMoveFeatureKey key, double value)
    {
        collectedFeatures_[key] = value;
    }

    double feature(EvaluationMoveFeatureKey key) const
    {
        auto it = collectedFeatures_.find(key);
        if (it == collectedFeatures_.end())
            return 0;

        return it->second;
    }

    const std::map<EvaluationMoveFeatureKey, double>& collectedFeatures() const { return collectedFeatures_; }

private:
    std::map<EvaluationMoveFeatureKey, double> collectedFeatures_;
};

class MidEvaluator : public EvaluatorBase {
public:
    explicit MidEvaluator(const PatternBook& patternBook) :
        EvaluatorBase(patternBook) {}

    MidEvalResult eval(const RefPlan&, const CoreField& currentField, double score);
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
class Evaluator : public EvaluatorBase {
public:
    // Don't take ownership of |sc|.
    Evaluator(const PatternBook& patternBook, ScoreCollector* sc) :
        EvaluatorBase(patternBook),
        sc_(sc) {}

    void eval(const RefPlan&, const KumipuyoSeq&, int currentFrameId, int maxIteration,
              const PlayerState& me, const PlayerState& enemy,
              const MidEvalResult&, bool fast, bool usesRensaHandTree, const GazeResult&);

    // ----------------------------------------------------------------------

    void evalStrategy(const RefPlan&, int currentFrameId, int rensaTreeValue,
                      const PlayerState& me, const PlayerState& enemy, const GazeResult&,
                      const MidEvalResult&);

    void evalFallenOjama(int fallenOjama);

    void evalMidEval(const MidEvalResult&);

private:
    CollectedCoef calculateDefaultCoef(const PlayerState& me, const PlayerState& enemy) const;

    ScoreCollector* sc_;
};

#endif // CPU_MAYAH_EVALUATOR_H_
