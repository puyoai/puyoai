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
    typedef std::function<void (const CoreField& complementedField,
                                const ColumnPuyoList& complementedColumnPuyoList)> ComplementCallback;
    typedef std::function<void (const CoreField& fieldAfterRensa,
                                const RensaResult& rensaResult,
                                const ColumnPuyoList& complementedPuyoList)> RensaCallback;
    template<typename TrackResult>
    using TrackedRensaCallback = std::function<void (const CoreField& fieldAfterRensa,
                                                     const RensaResult& rensaresult,
                                                     const ColumnPuyoList& complementedPuyos,
                                                     const TrackResult& trackResult)>;
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
                       const ComplementCallback& callback);

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

    // Complements at most |maxKeyPuyos| key puyos.
    // Callback is void callback(const CoreField&, const ColumnPuyoList&).
    // The complemented field and complemented ColumnPuyoList are passed.
    // Any puyo won't be erased when key puyos are complemented.
    template<typename Callback>
    static void complementKeyPuyos(const CoreField&,
                                   const RensaDetectorStrategy&,
                                   int maxKeyPuyos,
                                   Callback);

    // Complements at most |maxKeyPuyos| key puyos.
    // Also, complements fire puyos.
    // The complemented field and complemented ColumnPuyoList are passed.
    // Any puyo won't be erased when key puyos are complemented.
    // You need to fire your rensa.
    // Callback should be convertible to ComplementCallback.
    template<typename Callback>
    static void detectWithAddingKeyPuyos(const CoreField&,
                                         const RensaDetectorStrategy&,
                                         int maxKeyPuyos,
                                         Callback);

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

private:
    template<typename Callback>
    static void complementKeyPuyosInternal(CoreField& currentField,
                                           ColumnPuyoList& currentKeyPuyos,
                                           const RensaDetectorStrategy& strategy,
                                           int leftX,
                                           int restAdded,
                                           Callback callback);
};

#include "core/algorithm/rensa_detector_inl.h"

#endif // CORE_ALGORITHM_RENSA_DETECTOR_H_
