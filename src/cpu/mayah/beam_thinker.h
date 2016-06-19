#ifndef CPU_MAYAH_BEAM_THINKER_H_
#define CPU_MAYAH_BEAM_THINKER_H_

#include <mutex>

#include "base/executor.h"
#include "core/client/ai/drop_decision.h"
#include "core/core_field.h"
#include "core/kumipuyo_seq.h"
#include "core/player_state.h"

class BeamThinker {
public:
    explicit BeamThinker(Executor* executor) : executor_(executor) {}

    DropDecision think(int frame_id, const CoreField& field, const KumipuyoSeq& seq,
                       const PlayerState& me, const PlayerState& enemy, bool fast) const;

private:
    Executor* executor_;

    mutable std::mutex mu_;  // for cout
};

#endif // CPU_MAYAH_BEAM_THINKER_H_
