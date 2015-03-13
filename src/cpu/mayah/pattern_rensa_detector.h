#ifndef CPU_MAYAH_PATTERN_RENSA_DETECTOR_H_
#define CPU_MAYAH_PATTERN_RENSA_DETECTOR_H_

#include "core/column_puyo_list.h"
#include "core/core_field.h"

#include "pattern_book.h"

class PatternRensaDetector {
public:
    explicit PatternRensaDetector(const PatternBook& patternBook) : patternBook_(patternBook) {}

    typedef std::function<void (const CoreField&,
                                const RensaResult&,
                                const ColumnPuyoList& keyPuyos,
                                const ColumnPuyoList& firePuyos,
                                const RensaTrackResult&,
                                double patternScore)> Callback;

    void iteratePossibleRensas(const CoreField&,
                               const std::vector<int>& matchableIds,
                               int maxIteration,
                               const Callback&) const;

private:
    void iteratePossibleRensasInternal(const CoreField& originalField,
                                       const RensaDetectorStrategy&,
                                       int currentChains,
                                       const CoreField& currentField,
                                       const CoreField::SimulationContext& currentFieldContext,
                                       const ColumnPuyo& firePuyo,
                                       const ColumnPuyoList& keyPuyos,
                                       int restIteration,
                                       int restUnusedVariables,
                                       double sumPatternScore,
                                       const Callback& callback) const;

    bool checkRensa(const CoreField& originalField,
                    const RensaDetectorStrategy&,
                    int currentChains,
                    const ColumnPuyo& firePuyo,
                    const ColumnPuyoList& keyPuyos,
                    double sumPatternScore,
                    bool prohibits[FieldConstant::MAP_WIDTH],
                    const Callback& callback) const;

    const PatternBook& patternBook_;
};

#endif
