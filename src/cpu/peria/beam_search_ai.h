#ifndef CPU_SMAPLE_BEAM_SEARCH_AI_
#define CPU_SMAPLE_BEAM_SEARCH_AI_

#include <cstdint>
#include <string>
#include <unordered_set>

#include "core/client/ai/ai.h"

struct RensaResult;
class RefPlan;

namespace sample {

struct SearchState;

class BeamSearchAI : public AI {
  using uint64 = std::uint64_t;
  using int64 = std::int64_t;
 public:
  BeamSearchAI(const std::string& name) : AI(name) {}
  virtual ~BeamSearchAI() {}

  virtual DropDecision think(int frame_id, const CoreField& field, const KumipuyoSeq& seq,
                             const PlayerState&, const PlayerState&, bool fast) const;

 private:
  SearchState search(const CoreField& field, const KumipuyoSeq& vseq, int search_turns) const;

  void generateNextStates(const SearchState& state, int from, const Kumipuyo& kumi,
                          std::unordered_set<uint64>& visited,
                          std::vector<SearchState>& states) const;

  virtual bool skipRensaPlan(const RensaResult& result) const = 0;
  virtual SearchState generateNextRensaState(const CoreField& field, int from, const SearchState& state, const RefPlan& plan) const = 0;
  virtual SearchState generateNextNonRensaState(const CoreField& field, int from, const SearchState& state, const RefPlan& plan, int expect) const = 0;
  virtual bool shouldUpdateState(const SearchState& orig, const SearchState& res) const = 0;
};

}  // namespace sample

#endif  // CPU_SMAPLE_BEAM_SEARCH_AI_
