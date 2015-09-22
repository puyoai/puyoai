#ifndef CORE_RENSA_RENSA_DETECTOR_H_
#define CORE_RENSA_RENSA_DETECTOR_H_

#include <functional>

#include "base/base.h"
#include "core/rensa/rensa_detector_strategy.h"
#include "core/core_field.h"
#include "core/field_constant.h"
#include "core/rensa_tracker/rensa_last_vanished_position_tracker.h"

class ColumnPuyoList;
struct RensaResult;

enum class PurposeForFindingRensa {
    FOR_FIRE,
    FOR_KEY,
};

// RensaDetector is a set of functions to find a rensa from the specified field.
// Using detectIteratively() is recommended for most cases.
class RensaDetector {
public:
    typedef std::function<void (CoreField&& complementedField,
                                const ColumnPuyoList& complementedColumnPuyoList)> ComplementCallback;

    typedef std::function<RensaResult (CoreField&& complementedField,
                                       const ColumnPuyoList& complementedColumnPuyoList)> RensaSimulationCallback;

    // Detects a rensa from the field with the specified strategy.
    static void detectSingle(const CoreField&,
                             const RensaDetectorStrategy&,
                             const ComplementCallback&);

    // Detects a rensa iteratively from CoreField.
    // Algorithm is like the following (not accurate):
    // 1. Detects a rensa.
    // 2. Try to detect another rensa after the field where the previous rensa is finished.
    // 3. Complement 2's ColumnPuyoList, and 1's ColumnPuyoList, and check the size of rensa.
    // Do (2)-(3) |maxIteration - 1| times.
    static void detectIteratively(const CoreField&,
                                  const RensaDetectorStrategy&,
                                  int maxIteration,
                                  const RensaSimulationCallback&);

    // Finds 2-double (or more).
    static void detectSideChain(const CoreField&,
                                const RensaDetectorStrategy&,
                                const ComplementCallback&);

    // ----------------------------------------------------------------------
    // Don't use the following functions without understanding the algorithm.

    // Detects rensa by DROP strategy.
    static void detectByDropStrategy(const CoreField&,
                                     const bool prohibits[FieldConstant::MAP_WIDTH],
                                     PurposeForFindingRensa,
                                     int maxComplementPuyos,
                                     int maxPuyoHeight,
                                     const ComplementCallback&);
    // Detects rensa by FLOAT strategy.
    static void detectByFloatStrategy(const CoreField&,
                                      const bool prohibits[FieldConstant::MAP_WIDTH],
                                      int maxComplementPuyos,
                                      int maxPuyoHeight,
                                      const ComplementCallback&);
    // Detects rensa by EXTEND strategy.
    static void detectByExtendStrategy(const CoreField&,
                                       const bool prohibits[FieldConstant::MAP_WIDTH],
                                       int maxComplementPuyos,
                                       int maxPuyoHeight,
                                       const ComplementCallback&);

    // Detects a rensa from the field. The ColumnPuyoList to fire a rensa will be passed to
    // |callback|. Note that invalid column puyo list might be passed to |callback|.
    static void detect(const CoreField&,
                       const RensaDetectorStrategy&,
                       PurposeForFindingRensa,
                       const bool prohibits[FieldConstant::MAP_WIDTH],
                       const ComplementCallback&);

    static void detectSideChainFromDetectedField(const CoreField& originalField,
                                                 const CoreField& detectedField,
                                                 const RensaDetectorStrategy&,
                                                 const ColumnPuyoList& firePuyoList,
                                                 const ComplementCallback&);

    // Complements key puyos on 13th row.
    static void complementKeyPuyosOn13thRow(const CoreField&,
                                            const bool allowsComplements[FieldConstant::MAP_WIDTH],
                                            const ComplementCallback&);

    // Creates prohibit array from the specified result.
    // prohibit array is used for pruning. If prohibits[x] is true, detect() won't add any puyo on column x.
    static void makeProhibitArray(const CoreField& originalField,
                                  const RensaDetectorStrategy& strategy,
                                  const RensaLastVanishedPositionTrackResult&,
                                  const ColumnPuyoList& firePuyos,
                                  bool prohibits[FieldConstant::MAP_WIDTH]);

private:
    static void detectIterativelyInternal(const CoreField& originalField,
                                          const RensaDetectorStrategy& strategy,
                                          const CoreField& currentField,
                                          int restIterations,
                                          const ColumnPuyoList& accumulatedKeyPuyos,
                                          const ColumnPuyoList& firstRensaFirePuyos,
                                          int currentTotalChains,
                                          const bool prohibits[FieldConstant::MAP_WIDTH],
                                          const RensaSimulationCallback&);

    static void complementKeyPuyos13thRowInternal(CoreField& currentField,
                                                  ColumnPuyoList& currentKeyPuyos,
                                                  const bool allowsComplements[FieldConstant::MAP_WIDTH],
                                                  int x,
                                                  const ComplementCallback&);
};

#endif // CORE_RENSA_RENSA_DETECTOR_H_
