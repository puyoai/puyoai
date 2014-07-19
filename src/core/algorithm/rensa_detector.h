#ifndef CORE_ALGORITHM_RENSA_DETECTOR_H_
#define CORE_ALGORITHM_RENSA_DETECTOR_H_

#include <functional>
#include <vector>

#include "core/field/core_field.h"
#include "core/algorithm/puyo_set.h"
#include "core/algorithm/rensa_info.h"

class KumipuyoSeq;

class RensaDetector {
public:
    enum class Mode {
        DROP,
        FLOAT,
    };

    // Finds rensa using |kumipuyos|.
    static std::vector<FeasibleRensaInfo> findFeasibleRensas(const CoreField&, const KumipuyoSeq&);

    // Finds rensa from the specified field. We put |maxKeyPuyo| puyos as key puyo.
    typedef std::function<void (const CoreField&, const RensaResult&,
                                const ColumnPuyoList&, const ColumnPuyoList&)> PossibleRensaCallback;
    static void iteratePossibleRensas(const CoreField&, int maxKeyPuyo, PossibleRensaCallback, Mode mode = Mode::DROP);

    // Same as iteratePossibleRensas with checking trackResult.
    typedef std::function<void (const CoreField&, const RensaResult&, const ColumnPuyoList&, const ColumnPuyoList&,
                                const RensaTrackResult&)> TrackedPossibleRensaCallback;
    static void iteratePossibleRensasWithTracking(const CoreField&, int maxKeyPuyos,
                                                  TrackedPossibleRensaCallback, Mode mode = Mode::DROP);
};

#endif
