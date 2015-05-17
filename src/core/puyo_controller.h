#ifndef CORE_PUYO_CONTROLLER_H_
#define CORE_PUYO_CONTROLLER_H_

#include "core/key_set_seq.h"

class CoreField;
class Decision;
class KumipuyoMovingState;

class PuyoController {
public:
    static bool isReachable(const CoreField&, const Decision&);
    static bool isReachableFrom(const CoreField&, const KumipuyoMovingState&, const Decision&);

    // Finds a key stroke to move puyo from |KumipuyoMovingState| to |Decision|.
    // When there is not such a way, the returned KeySetSeq would be empty sequence.
    static PrecedeKeySetSeq findKeyStroke(const CoreField&, const Decision&);
    static KeySetSeq findKeyStrokeFrom(const CoreField&, const KumipuyoMovingState&, const Decision&);

private:
    static KeySetSeq findKeyStrokeOnlineInternal(const CoreField&, const KumipuyoMovingState&, const Decision&);

    // Fast, but usable in limited situation.
    static PrecedeKeySetSeq findKeyStrokeFastpath(const CoreField&, const Decision&);
    // This is faster, but might output worse key stroke.
    static KeySetSeq findKeyStrokeOnline(const CoreField&, const KumipuyoMovingState&, const Decision&);
    // This is slow, but precise.
    static KeySetSeq findKeyStrokeByDijkstra(const CoreField&, const KumipuyoMovingState&, const Decision&);
};

#endif  // CORE_PUYO_CONTROLLER_H_
