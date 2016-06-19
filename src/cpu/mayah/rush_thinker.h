#ifndef CPU_MAYAH_BEAM_RUSH_THINKER_H_
#define CPU_MAYAH_BEAM_RUSH_THINKER_H_

#include <string>
#include <unordered_set>

#include "core/client/ai/ai.h"

struct RensaResult;
class RefPlan;

struct SearchState;

// BeamSearchThinker is a base AI to implement an AI with beam search algorithm.
// It uses some virtual methods to change its behavior.

class BeamSearchThinker {
    using uint64 = std::uint64_t;
    using int64 = std::int64_t;
public:
    BeamSearchThinker() {}
    virtual ~BeamSearchThinker() {}

    virtual DropDecision think(int frame_id, const CoreField& field, const KumipuyoSeq& seq,
                               const PlayerState&, const PlayerState&, bool fast) const;

private:
    SearchState search(const CoreField& field, const KumipuyoSeq& vseq, int search_turns) const;

    void generateNextStates(const SearchState& state, int from, const Kumipuyo& kumi,
                            std::unordered_set<uint64>& visited,
                            std::vector<SearchState>& states) const;

    // pure virtual methods to change the behavior.
    virtual bool skipRensaPlan(const RensaResult& result) const = 0;
    virtual SearchState generateNextRensaState(const CoreField& field, int from, const SearchState& state, const RefPlan& plan) const = 0;
    virtual SearchState generateNextNonRensaState(const CoreField& field, int from, const SearchState& state, const RefPlan& plan, int expect) const = 0;
    virtual bool shouldUpdateState(const SearchState& orig, const SearchState& res) const = 0;
};

class RushThinker : public BeamSearchThinker {
public:
    RushThinker();

    DropDecision think(int frame_id, const CoreField& field, const KumipuyoSeq& seq,
                       const PlayerState&, const PlayerState&, bool) const override;

private:
    bool skipRensaPlan(const RensaResult& result) const override;
    SearchState generateNextRensaState(const CoreField& field, int from, const SearchState& state, const RefPlan& plan) const override;
    SearchState generateNextNonRensaState(const CoreField& field, int from, const SearchState& state, const RefPlan& plan, int expect) const override;
    bool shouldUpdateState(const SearchState& orig, const SearchState& res) const override;
};

#endif // CPU_MAYAH_BEAM_RUSH_THINKER_H_
