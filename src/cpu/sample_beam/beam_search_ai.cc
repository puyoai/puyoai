#include "cpu/sample_beam/beam_search_ai.h"

#include <array>
#include <limits>
#include <sstream>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/base.h"
#include "base/time.h"
#include "core/plan/plan.h"
#include "core/rensa/rensa_detector.h"
#include "core/core_field.h"
#include "core/kumipuyo_seq_generator.h"

#define RECORD_RANK_LOG 0

#if RECORD_RANK_LOG
#include <iostream>
#endif

DEFINE_int32(max_simulate, 5, "The maximum number to iterate simulations.");
DEFINE_int32(search_turns, 20, "Maximum turns to search in a simulation");
DEFINE_int64(time_limit, 150, "Time limit to think. [ms]");
DEFINE_int32(beam_width, 400, "Size of recorded states in beam search");

DEFINE_string(type, "2dub", "Type of AI. Choose \"2dub\" or \"full\".");

namespace sample {

struct SearchState {
  CoreField field;
  Decision decision;
  int from;
  std::array<int, 3> features;
  // Fill: [0: # of ojama, 1: expected score, 2: -frames]
  // 2Dub: [0: # of 2dub, 1: # of ojama, 2: expected score]
};

BeamFullAI::BeamFullAI() : BeamSearchAI("Full") {}
  
bool BeamFullAI::skipRensaPlan(const RensaResult&) const {
  return false;
}

SearchState BeamFullAI::generateNextRensaState(const CoreField& field, int from, const SearchState& state, const RefPlan& plan) const {
  int ojama = std::min(plan.score() / 70, 60);
  int neg_frame = state.features[2] - plan.totalFrames();

  SearchState ret;
  ret.field = field;
  ret.decision = (state.decision.x == 0) ? plan.decision(0) : state.decision;
  ret.from = from;
  ret.features[0] = ojama;
  ret.features[1] = 0;
  ret.features[2] = neg_frame;
  return ret;
}

SearchState BeamFullAI::generateNextNonRensaState(const CoreField& field, int from, const SearchState& state, const RefPlan& plan, int expect) const {
  int neg_frame = state.features[2] - plan.totalFrames();

  SearchState ret;
  ret.field = field;
  ret.decision = (state.decision.x == 0) ? plan.decision(0) : state.decision;
  ret.from = from;
  ret.features[0] = 0;
  ret.features[1] = expect;
  ret.features[2] = neg_frame;
  return ret;
}

bool BeamFullAI::shouldUpdateState(const SearchState& orig, const SearchState& res) const {
  if (res.features[0] > orig.features[0])
    return true;
  if (res.features[0] == orig.features[0] && res.features[2] > orig.features[2])
    return true;
  return false;
}

// -------------------------------------------------------------------

Beam2DubAI::Beam2DubAI() : BeamSearchAI("2Dub") {}

bool Beam2DubAI::skipRensaPlan(const RensaResult& result) const {
  return result.chains > 2 || result.score < 680;
}

SearchState Beam2DubAI::generateNextRensaState(const CoreField& field, int from, const SearchState& state, const RefPlan& plan) const {
  int ojama = plan.score() / 70;
  SearchState ret;
  ret.field = field;
  ret.decision = (state.decision.x == 0) ? plan.decision(0) : state.decision;
  ret.from = from;
  ret.features[0] = state.features[0] + 1;
  ret.features[1] = state.features[1] + ojama;
  ret.features[2] = 0;
  return ret;
}

SearchState Beam2DubAI::generateNextNonRensaState(const CoreField& field, int from, const SearchState& state, const RefPlan& plan, int expect) const {
  SearchState ret;
  ret.field = field;
  ret.decision = (state.decision.x == 0) ? plan.decision(0) : state.decision;
  ret.from = from;
  ret.features[0] = state.features[0];
  ret.features[1] = state.features[1];
  ret.features[2] = expect;
  return ret;
}

bool Beam2DubAI::shouldUpdateState(const SearchState& orig, const SearchState& res) const {
  if (res.features[0] > orig.features[0])
    return true;
  if (res.features[0] == orig.features[0] && res.features[1] > orig.features[1])
    return true;
  return false;
}

// ===================================================================

DropDecision BeamSearchAI::think(
    int frame_id, const CoreField& field, const KumipuyoSeq& seq,
    const PlayerState&, const PlayerState&, bool) const {
  int64 start_time = currentTimeInMillis();
  int64 now = start_time;

  std::vector<int> features[7][4];  // [1<=x<=6][0<=r<4]

  int num_simulate = 0;
  for (num_simulate = 0; num_simulate < FLAGS_max_simulate; now = currentTimeInMillis()) {
    if (now >= start_time + FLAGS_time_limit)
      break;

    int search_turns = FLAGS_search_turns;
    KumipuyoSeq vseq = seq;  // 'v' stands for "virtual"
    vseq.append(KumipuyoSeqGenerator::generateRandomSequenceWithSeed(search_turns - vseq.size(), frame_id + num_simulate));
    SearchState state = search(field, vseq, search_turns);
    ++num_simulate;
    if (state.features[0] < 0)
      continue;

    const Decision& decision = state.decision;
    auto& list = features[decision.axisX()][decision.rot()];
    list.push_back(state.features[0]);
    if (static_cast<int>(list.size()) >= (FLAGS_max_simulate + 1) / 2)
      break;
  }

  std::vector<std::pair<Decision, double>> decisions;
  for (int x = 1; x <= 6; ++x) {
    for (int r = 0; r < 4; ++r) {
      if (features[x][r].empty())
        continue;
      const auto& list = features[x][r];
      double sum = std::accumulate(list.begin(), list.end(), 0);
      double expect = sum / list.size();
      decisions.push_back(std::make_pair(Decision(x, r), expect));
    }
  }
  std::sort(decisions.begin(), decisions.end(),
            [](const std::pair<Decision, double>& a, const std::pair<Decision, double>& b) {
              return a.second > b.second;
            });

  int64 end_time = currentTimeInMillis();

  std::ostringstream oss;
  oss << "Time:" << (end_time - start_time) << "[ms]_/_" << "Simulates:" << num_simulate << ",";
  for (auto& dd : decisions) {
    oss << "(" << dd.first.x << "-" << dd.first.r << ":" << dd.second << ")";
  }
  Decision best = (decisions.empty()) ? Decision(6, 0) : (decisions.begin()->first);
  
  return DropDecision(best, oss.str());
}

SearchState BeamSearchAI::search(
    const CoreField& field, const KumipuyoSeq& vseq, int search_turns) const {
  CHECK_GE(vseq.size(), search_turns);

  std::vector<std::vector<SearchState>> q_states(search_turns + 1);
  
  SearchState init_state;
  init_state.field = field;
  init_state.decision = Decision(0, 0);
  init_state.features[0] = 0;
  init_state.features[1] = 0;
  init_state.features[2] = std::numeric_limits<int>::min();

  q_states[0].push_back(init_state);
  for (int t = 0; t < search_turns; ++t) {
    std::unordered_set<uint64> visited;
    const auto& que = q_states[t];
    std::vector<SearchState>& next_states = q_states[t + 1];
    for (size_t i = 0; i < que.size(); ++i)
      generateNextStates(que[i], i, vseq.get(t), visited, next_states);

    if (next_states.empty())
      break;
    std::sort(next_states.begin(), next_states.end(),
              [](const SearchState& a, const SearchState& b) {
                if (a.features[0] == b.features[0]) {
                  if (a.features[1] == b.features[1])
                    return a.features[2] > b.features[2];
                  return a.features[1] > b.features[1];
                }
                return a.features[0] > b.features[0];
              });
    if (static_cast<int>(next_states.size()) >= FLAGS_beam_width)
      next_states.erase(next_states.begin() + FLAGS_beam_width, next_states.end());

    const SearchState best = *next_states.begin();
    if (std::all_of(next_states.begin(), next_states.end(),
        [&best](const SearchState& s){ return s.decision == best.decision; })) {
      search_turns = t + 1;
      break;
    }
  }

#if RECORD_RANK_LOG
  int bt = 0;
  int bi = 0;
#endif
  SearchState result = q_states[0][0];
  for (int t = 0; t < search_turns; ++t) {
    for (size_t i = 0; i < q_states[t].size(); ++i) {
      const auto& s = q_states[t][i];
      if (shouldUpdateState(result, s)) {
#if RECORD_RANK_LOG
        bt = t;
        bi = i;
#endif
        result = s;
      }
    }
  }

#if RECORD_RANK_LOG
  // Record how the best hand's rank changed.
  std::vector<int> ranks;
  for (int t = bt; t > 0; --t) {
    ranks.push_back(bi);
    bi = q_states[t][bi].from;
  }
  std::reverse(ranks.begin(), ranks.end());
  std::ostringstream oss;
  for (int r : ranks)
    oss << " " << r;
  std::cerr << "[" << ranks.size() << "]" << oss.str() << "\n";
#endif

  return result;
}

void BeamSearchAI::generateNextStates(
    const SearchState& state, int from, const Kumipuyo& kumi,
    std::unordered_set<uint64>& visited, std::vector<SearchState>& states) const {
  const BeamSearchAI* th = this;
  auto callback = [&th, &state, &from, &visited, &states](const RefPlan& plan) {
    const CoreField field = plan.field();
    RensaResult result = plan.rensaResult();

    uint64 h = field.hash();
    if (!visited.insert(h).second)
      return;

    if (plan.isRensaPlan()) {
      if (th->skipRensaPlan(result))
        return;

      SearchState next = th->generateNextRensaState(field, from, state, plan);
      states.push_back(next);

      return;
    }

    // Expected number of Ojama puyos to send in future.
    int expect = 0;
    auto detect_callback = [&expect](CoreField&& f, const ColumnPuyoList&) {
      RensaResult r = f.simulate();
      expect = std::max(expect, r.score);
    };
    bool prohibits[FieldConstant::MAP_WIDTH] {};
    RensaDetector::detectByDropStrategy(field, prohibits,
                                        PurposeForFindingRensa::FOR_FIRE, 2, 13,
                                        detect_callback);

    SearchState next = th->generateNextNonRensaState(field, from, state, plan, expect);
    states.push_back(next);
  };

  Plan::iterateAvailablePlans(state.field, {kumi}, 1, callback);
}

}  // namespace sample
