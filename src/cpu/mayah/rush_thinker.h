#ifndef CPU_MAYAH_BEAM_RUSH_THINKER_H_
#define CPU_MAYAH_BEAM_RUSH_THINKER_H_

#include <string>
#include <unordered_set>

#include "core/client/ai/ai.h"

struct RensaResult;
class RefPlan;

struct SearchState;

class RushThinker {
public:
    RushThinker();

    DropDecision think(int frame_id, const CoreField& field, const KumipuyoSeq& seq,
                       const PlayerState&, const PlayerState&, bool) const;

private:
    SearchState search(const CoreField& field, const KumipuyoSeq& vseq, int search_turns) const;

    void generateNextStates(const SearchState& state, int from, const Kumipuyo& kumi,
                            std::unordered_set<std::uint64_t>& visited,
                            std::vector<SearchState>& states) const;

    SearchState generateNextRensaState(const CoreField& field, int from, const SearchState& state, const RefPlan& plan) const;
    SearchState generateNextNonRensaState(const CoreField& field, int from, const SearchState& state, const RefPlan& plan, int expect) const;
    bool shouldUpdateState(const SearchState& orig, const SearchState& res) const;
};

#endif // CPU_MAYAH_BEAM_RUSH_THINKER_H_
