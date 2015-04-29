#ifndef CORE_ALGORITHM_RENSA_DETECTOR_H_
#define CORE_ALGORITHM_RENSA_DETECTOR_H_

#include <functional>

#include "core/algorithm/rensa_detector_strategy.h"
#include "core/field_constant.h"

class ColumnPuyoList;
class CoreField;
class RensaChainTrackResult;
class RensaCoefResult;
class RensaVanishingPositionResult;
struct RensaResult;

enum class PurposeForFindingRensa {
    FOR_FIRE,
    FOR_KEY,
};

// RensaDetector is a set of functions to find a rensa from the specified field.
// Using iteratePossibleRensasIteratively() is recommended for most cases.
// TODO(mayah): Simplify this class.
class RensaDetector {
public:
    typedef std::function<void (CoreField*, const ColumnPuyoList&)> SimulationCallback;
    typedef std::function<void (const CoreField&,
                                const RensaResult&,
                                const ColumnPuyoList&)> RensaCallback;
    template<typename TrackResult>
    using TrackedRensaCallback = std::function<void (const CoreField&,
                                                     const RensaResult&,
                                                     const ColumnPuyoList&,
                                                     const TrackResult&)>;
    typedef TrackedRensaCallback<RensaChainTrackResult> TrackedPossibleRensaCallback;
    typedef TrackedRensaCallback<RensaCoefResult> CoefPossibleRensaCallback;
    typedef TrackedRensaCallback<RensaVanishingPositionResult> VanishingPositionPossibleRensaCallback;

    // Detects a rensa from the field. The ColumnPuyoList to fire a rensa will be passed to
    // |callback|. Note that invalid column puyo list might be passed to |callback|.
    // The field that puyo list is added is also passed to DetectionCallback.
    static void detect(const CoreField&,
                       const RensaDetectorStrategy&,
                       PurposeForFindingRensa,
                       const bool prohibits[FieldConstant::MAP_WIDTH],
                       const SimulationCallback& callback);

    // TODO(mayah): Consider simplify these methods.

    // Finds rensa from the specified field. We put |maxKeyPuyo| puyos as key puyo.
    static void iteratePossibleRensas(const CoreField&,
                                      int maxKeyPuyo,
                                      const RensaDetectorStrategy&,
                                      const RensaCallback&);

    static void iteratePossibleRensasWithTracking(const CoreField&,
                                                  int maxKeyPuyos,
                                                  const RensaDetectorStrategy&,
                                                  const TrackedPossibleRensaCallback&);

    static void iteratePossibleRensasWithCoefTracking(const CoreField&,
                                                      int maxKeyPuyos,
                                                      const RensaDetectorStrategy&,
                                                      const CoefPossibleRensaCallback&);

    static void iteratePossibleRensasWithVanishingPositionTracking(const CoreField&,
                                                                   int maxKeyPuyos,
                                                                   const RensaDetectorStrategy&,
                                                                   const VanishingPositionPossibleRensaCallback&);

    // Finds a rensa from CoreField. TrackedPossibleRensaCallback is called for all detected rensas.
    // Algorithm is like the following (not accurate):
    // 1. Vanish puyos from CoreField.
    // 2. After (1), Also vanish puyos from CoreField. The complemneted puyos to fire this rensa are considered
    //    as key puyos. Iterate this (maxIteration - 1) times.
    // 3. Complement all puyos for (1) and (2), and check the rensa is not corrupted. If not corrupted,
    //    call the callback.
    static void iteratePossibleRensasIteratively(const CoreField&,
                                                 int maxIteration,
                                                 const RensaDetectorStrategy&,
                                                 const TrackedPossibleRensaCallback&);

    static void makeProhibitArray(const RensaResult&,
                                  const RensaChainTrackResult&,
                                  const CoreField& originalField,
                                  const ColumnPuyoList& firePuyos,
                                  bool prohibits[FieldConstant::MAP_WIDTH]);
};

#endif
