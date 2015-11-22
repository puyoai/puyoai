#ifndef CPU_SMAPLE_BEAM_BEAM_SEARCH_AI_
#define CPU_SMAPLE_BEAM_BEAM_SEARCH_AI_

// BeamSearchAI is a skelton AI to implement AIs using beam search algorithm.
// This file also creates 2 different type AIs ineriting from BeamSearchAI.

#include <string>
#include <unordered_set>

#include "core/client/ai/ai.h"

struct RensaResult;
class RefPlan;

namespace sample {

struct SearchState;

// BeamSearchAI is a base AI to implement an AI with beam search algorithm.
// It uses some virtual methods to change its behavior.

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

  // pure virtual methods to change the behavior.
  virtual bool skipRensaPlan(const RensaResult& result) const = 0;
  virtual SearchState generateNextRensaState(const CoreField& field, int from, const SearchState& state, const RefPlan& plan) const = 0;
  virtual SearchState generateNextNonRensaState(const CoreField& field, int from, const SearchState& state, const RefPlan& plan, int expect) const = 0;
  virtual bool shouldUpdateState(const SearchState& orig, const SearchState& res) const = 0;
};

// Type specified AIs ------------------------------------------------

// BeamFullAI simply tries to fill enemy's field with Ojama puyos.
// As a result, it fires 5-rensa or 4-dub frequently.
class BeamFullAI final : public BeamSearchAI {
public:
  BeamFullAI();

private:
  bool skipRensaPlan(const RensaResult&) const override;
  SearchState generateNextRensaState(const CoreField& field, int from, const SearchState& state, const RefPlan& plan) const override;
  SearchState generateNextNonRensaState(const CoreField& field, int from, const SearchState& state, const RefPlan& plan, int expect) const override;
  bool shouldUpdateState(const SearchState& orig, const SearchState& res) const override;
};

// Beam2DubAI tries to fire 2-double rensa ASAP.
class Beam2DubAI final : public BeamSearchAI {
public:
  Beam2DubAI();

private:
  DropDecision think(int frame_id, const CoreField& field, const KumipuyoSeq& seq,
                     const PlayerState&, const PlayerState&, bool) const override;

  bool skipRensaPlan(const RensaResult& result) const override;
  SearchState generateNextRensaState(const CoreField& field, int from, const SearchState& state, const RefPlan& plan) const override;
  SearchState generateNextNonRensaState(const CoreField& field, int from, const SearchState& state, const RefPlan& plan, int expect) const override;
  bool shouldUpdateState(const SearchState& orig, const SearchState& res) const override;
};

}  // namespace sample

#endif  // CPU_SMAPLE_BEAM_BEAM_SEARCH_AI_
