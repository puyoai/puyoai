#ifndef CORE_PUYO_CONTROLLER_H_
#define CORE_PUYO_CONTROLLER_H_

#include <glog/logging.h>
#include <string>
#include <stdio.h>
#include <tuple>
#include <vector>

#include "core/constant.h"
#include "core/key.h"
#include "core/key_set.h"
#include "core/kumipuyo.h"
#include "core/kumipuyo_moving_state.h"
#include "core/kumipuyo_pos.h"
#include "core/puyo_color.h"

class CoreField;
class Decision;
class KumipuyoPos;
class PlainField;

class PuyoController {
public:
    static bool isReachable(const PlainField&, const Decision&);
    static bool isReachableFrom(const PlainField&, const KumipuyoMovingState&, const Decision&);

    // Finds a key stroke to move puyo from |KumipuyoMovingState| to |Decision|.
    // When there is not such a way, the returned KeySetSeq would be empty sequence.
    static KeySetSeq findKeyStroke(const CoreField&, const KumipuyoMovingState&, const Decision&);

    // TODO(mayah): Move these to private section?
    // Fast, but usable in limited situation.
    static KeySetSeq findKeyStrokeFastpath(const CoreField&, const Decision&);
    // This is faster, but might output worse key stroke.
    static KeySetSeq findKeyStrokeOnline(const PlainField&, const KumipuyoMovingState&, const Decision&);
    // This is slow, but precise.
    static KeySetSeq findKeyStrokeByDijkstra(const PlainField&, const KumipuyoMovingState&, const Decision&);

private:
    static bool isReachableFastpath(const PlainField&, const Decision&);

    static KeySetSeq findKeyStrokeOnlineInternal(const PlainField&, const KumipuyoMovingState&, const Decision&);
};

#endif  // CORE_PUYO_CONTROLLER_H_
