#ifndef CPU_MAYAH_RENSA_EVALUATOR_H_
#define CPU_MAYAH_RENSA_EVALUATOR_H_

#include "evaluation_feature.h"
#include "score_collector.h"

class CoreField;
class ColumnPuyoList;
class FieldBits;
class PatternBook;
struct PlayerState;
class RefPlan;
struct RensaResult;

template<typename ScoreCollector>
class RensaEvaluator {
public:
    // Don't take ownership of |sc|.
    RensaEvaluator(const PatternBook& patternBook, ScoreCollector* sc) :
        patternBook_(patternBook),
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
    const PatternBook& patternBook() const { return patternBook_; }

    const PatternBook& patternBook_;
    ScoreCollector* sc_;
};

#endif
