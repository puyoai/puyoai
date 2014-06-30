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

    // Finds possible rensas from the specified field.
    // Before firing a rensa, we will put key puyos (maxKeyPuyo at max).
    static std::vector<PossibleRensaInfo> findPossibleRensas(
        const CoreField&, int maxKeyPuyo = 0, Mode mode = Mode::DROP);

    // Same as findPossibleRensas. However, we return tracking info also.
    static std::vector<TrackedPossibleRensaInfo> findPossibleRensasWithTracking(
        const CoreField&, int maxKeyPuyo = 0, Mode mode = Mode::DROP);

    typedef std::function<void (CoreField*, int, PuyoColor, int)> SimulationCallback;

    static void findRensas(const CoreField&, Mode mode, SimulationCallback callback);
};

#endif
