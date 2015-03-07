#ifndef CORE_ALGORITHM_RENSA_DETECTOR_H_
#define CORE_ALGORITHM_RENSA_DETECTOR_H_

#include <functional>
#include <vector>

#include "core/algorithm/puyo_set.h"
#include "core/core_field.h"

class KumipuyoSeq;
class RensaRefSequence;

class RensaDetectorStrategy {
public:
    enum class Mode {
        DROP, FLOAT, EXTEND,
    };

    RensaDetectorStrategy(Mode mode,
                          int maxNumOfComplementPuyosForKey,
                          int maxNumOfComplementPuyosForFire,
                          bool allowsPuttingKeyPuyoOn13thRow) :
        mode_(mode),
        maxNumOfComplementPuyosForKey_(maxNumOfComplementPuyosForKey),
        maxNumOfComplementPuyosForFire_(maxNumOfComplementPuyosForFire),
        allowsPuttingKeyPuyoOn13thRow_(allowsPuttingKeyPuyoOn13thRow)
    {
    }

    static RensaDetectorStrategy defaultFloatStrategy() { return RensaDetectorStrategy(Mode::FLOAT, 3, 3, true); }
    static RensaDetectorStrategy defaultDropStrategy() { return RensaDetectorStrategy(Mode::DROP, 3, 3, true); }
    static RensaDetectorStrategy defaultExtendStrategy() { return RensaDetectorStrategy(Mode::EXTEND, 3, 3, false); }

    Mode mode() const { return mode_; }
    int maxNumOfComplementPuyosForKey() const { return maxNumOfComplementPuyosForKey_; }
    int maxNumOfComplementPuyosForFire() const { return maxNumOfComplementPuyosForFire_; }
    bool allowsPuttingKeyPuyoOn13thRow() const { return allowsPuttingKeyPuyoOn13thRow_; }

private:
    Mode mode_;
    int maxNumOfComplementPuyosForKey_;
    int maxNumOfComplementPuyosForFire_;
    bool allowsPuttingKeyPuyoOn13thRow_;
};

enum class PurposeForFindingRensa {
    FOR_FIRE,
    FOR_KEY,
};

class RensaDetector {
public:
    typedef std::function<void (CoreField*, const ColumnPuyoList&)> SimulationCallback;
    // Detects a rensa from the field. The ColumnPuyoList to fire a rensa will be passed to
    // |callback|. Note that invalid column puyo list might be passed to |callback|.
    // The field that puyo list is added is also passed to DetectionCallback.
    static void detect(const CoreField&,
                       const RensaDetectorStrategy&,
                       PurposeForFindingRensa,
                       const bool prohibits[FieldConstant::MAP_WIDTH],
                       const SimulationCallback& callback);

    typedef std::function<void (const CoreField& fieldAfterRensa,
                                const RensaResult&,
                                const ColumnPuyoList&)> RensaCallback;
    // Detects a rensa from the field. We don't add key puyos etc.
    static void detectSingle(const CoreField&, const RensaDetectorStrategy&, RensaCallback);

    // Finds rensa from the specified field. We put |maxKeyPuyo| puyos as key puyo.
    typedef std::function<void (const CoreField&,
                                const RensaResult&,
                                const ColumnPuyoList& keyPuyos,
                                const ColumnPuyoList& firePuyos)> PossibleRensaCallback;
    static void iteratePossibleRensas(const CoreField&,
                                      int maxKeyPuyo,
                                      const RensaDetectorStrategy&,
                                      PossibleRensaCallback);

    // Same as iteratePossibleRensas with checking trackResult.
    typedef std::function<void (const CoreField&,
                                const RensaResult&,
                                const ColumnPuyoList& keyPuyos,
                                const ColumnPuyoList& firePuyos,
                                const RensaTrackResult&)> TrackedPossibleRensaCallback;
    static void iteratePossibleRensasWithTracking(const CoreField&,
                                                  int maxKeyPuyos,
                                                  const RensaDetectorStrategy&,
                                                  TrackedPossibleRensaCallback);

    typedef std::function<void (const CoreField&,
                                const RensaResult&,
                                const ColumnPuyoList& keyPuyos,
                                const ColumnPuyoList& firePuyos,
                                const RensaCoefResult& coefResult)> CoefPossibleRensaCallback;
    static void iteratePossibleRensasWithCoefTracking(const CoreField&,
                                                      int maxKeyPuyos,
                                                      const RensaDetectorStrategy&,
                                                      CoefPossibleRensaCallback);

    typedef std::function<void (const CoreField&,
                                const RensaResult&,
                                const ColumnPuyoList& keyPuyos,
                                const ColumnPuyoList& firePuyos,
                                const RensaVanishingPositionResult& coefResult)> VanishingPositionPossibleRensaCallback;
    static void iteratePossibleRensasWithVanishingPositionTracking(const CoreField&,
                                                                   int maxKeyPuyos,
                                                                   const RensaDetectorStrategy&,
                                                                   VanishingPositionPossibleRensaCallback);

    // Without adding key puyos, we find rensas iteratively.
    typedef std::function<void (const CoreField&,
                                const RensaResult&,
                                const ColumnPuyoList& keyPuyos,
                                const ColumnPuyoList& firePuyos,
                                const RensaTrackResult&,
                                const RensaRefSequence&)> IterativePossibleRensaCallback;
    static void iteratePossibleRensasIteratively(const CoreField&,
                                                 int maxIteration,
                                                 const RensaDetectorStrategy&,
                                                 IterativePossibleRensaCallback);

    static void makeProhibitArray(const RensaResult&,
                                  const RensaTrackResult&,
                                  const CoreField& originalField,
                                  const ColumnPuyoList& firePuyos,
                                  bool prohibits[FieldConstant::MAP_WIDTH]);
};

#endif
