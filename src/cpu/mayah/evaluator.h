#ifndef CPU_MAYAH_EVALUATOR_H_
#define CPU_MAYAH_EVALUATOR_H_

#include <map>
#include <vector>

#include "evaluation_feature.h"
#include "pattern_book.h"
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
    EvaluatorBase(const PatternBook& patternBook, const NewPatternBook& newPatternBook) :
        patternBook_(patternBook), newPatternBook_(newPatternBook) {}

    const PatternBook& patternBook() const { return patternBook_; }
    const NewPatternBook& newPatternBook() const { return newPatternBook_; }

private:
    const PatternBook& patternBook_;
    const NewPatternBook& newPatternBook_;
};

class PreEvalResult {
public:
    PreEvalResult() {}

    const std::vector<int>& matchablePatternIds() const { return matchablePatternIds_; }

    std::vector<int>* mutableMatchablePatternIds() { return &matchablePatternIds_; }

private:
    std::vector<int> matchablePatternIds_;
};

class PreEvaluator : public EvaluatorBase {
public:
    PreEvaluator(const PatternBook& patternBook, const NewPatternBook& newPatternBook) :
        EvaluatorBase(patternBook, newPatternBook) {}

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
    MidEvaluator(const PatternBook& patternBook, const NewPatternBook& newPatternBook) :
        EvaluatorBase(patternBook, newPatternBook) {}

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
class RensaEvaluator : public EvaluatorBase {
public:
    // Don't take ownership of |sc|.
    RensaEvaluator(const PatternBook& patternBook, const NewPatternBook& newPatternBook, ScoreCollector* sc) :
        EvaluatorBase(patternBook, newPatternBook),
        sc_(sc) {}

    void evalPatternScore(const ColumnPuyoList& puyosToComplement, double patternScore, int chains);
    void evalRensaScore(double score, double virtualScore);
    void evalRensaChainFeature(const RensaResult&, const ColumnPuyoList&);
    void evalRensaGarbage(const CoreField& fieldAfterRensa);
    void evalFirePointTabooFeature(const CoreField&, const FieldBits& ignitionPuyoBits);
    void evalRensaIgnitionHeightFeature(const CoreField&, const FieldBits& ignitionPuyoBits);
    void evalRensaConnectionFeature(const CoreField& fieldAfterDrop);
    void evalRensaRidgeHeight(const CoreField&);
    void evalRensaValleyDepth(const CoreField&);
    void evalRensaFieldUShape(const CoreField&);
    void evalComplementationBias(const ColumnPuyoList&);
    void evalRensaStrategy(const RefPlan&, const RensaResult&, const ColumnPuyoList&,
                           int currentFrameId, const PlayerState& me, const PlayerState& enemy);
private:
    ScoreCollector* sc_;
};

template<typename ScoreCollector>
class Evaluator : public EvaluatorBase {
public:
    // Don't take ownership of |sc|.
    Evaluator(const PatternBook& patternBook, const NewPatternBook& newPatternBook, ScoreCollector* sc) :
        EvaluatorBase(patternBook, newPatternBook),
        sc_(sc) {}

    void eval(const RefPlan&, const KumipuyoSeq&, int currentFrameId, int maxIteration,
              const PlayerState& me, const PlayerState& enemy,
              const PreEvalResult&, const MidEvalResult&, bool fast, bool usesRensaHandTree, const GazeResult&);

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
