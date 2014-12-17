#ifndef CORE_ALGORITHM_RENSA_DETECTOR_H_
#define CORE_ALGORITHM_RENSA_DETECTOR_H_

#include <functional>
#include <vector>

#include "core/algorithm/puyo_set.h"
#include "core/algorithm/rensa_info.h"
#include "core/core_field.h"

class KumipuyoSeq;
class RensaRefSequence;

class RensaDetectorStrategy {
public:
    enum class Mode {
        DROP, FLOAT
    };

    explicit RensaDetectorStrategy(Mode mode) : mode_(mode) {}

    static const RensaDetectorStrategy& defaultFloatStrategy();
    static const RensaDetectorStrategy& defaultDropStrategy();

    Mode mode() const { return mode_; }

private:
    Mode mode_;
};

class RensaDetector {
public:
    // Finds rensa using |kumipuyos|.
    static std::vector<FeasibleRensaInfo> findFeasibleRensas(const CoreField&, const KumipuyoSeq&);

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
    typedef std::function<void (const CoreField&, const RensaResult&,
                                const ColumnPuyoList& keyPuyos, const ColumnPuyoList& firePuyos,
                                const RensaTrackResult&)> TrackedPossibleRensaCallback;
    static void iteratePossibleRensasWithTracking(const CoreField&,
                                                  int maxKeyPuyos,
                                                  const RensaDetectorStrategy&,
                                                  TrackedPossibleRensaCallback);

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
};

#endif
