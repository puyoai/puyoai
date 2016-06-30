#ifndef CPU_MAYAH_BEAM_SIDE_THINKER_H_
#define CPU_MAYAH_BEAM_SIDE_THINKER_H_

#include <string>
#include <unordered_set>

#include "core/client/ai/ai.h"

struct RensaResult;
class RefPlan;

class SideThinker {
public:
    SideThinker();

    DropDecision think(int frame_id, const CoreField& field, const KumipuyoSeq& seq,
                       const PlayerState&, const PlayerState&, bool) const;
};

#endif // CPU_MAYAH_BEAM_SIDE_THINKER_H_
