#include <cstdint>
#include <sstream>
#include <unordered_set>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "base/base.h"
#include "base/time.h"
#include "core/algorithm/plan.h"
#include "core/algorithm/rensa_detector.h"
#include "core/client/ai/ai.h"
#include "core/core_field.h"
#include "core/kumipuyo_seq_generator.h"

DEFINE_int32(max_simulate, 5, "The maximum number to iterate simulations.");
DEFINE_int32(search_turns, 20, "Maximum turns to search in a simulation");
DEFINE_int64(time_limit, 150, "Time limit to think. [ms]");

DEFINE_int32(beam_width, 400, "Size of recorded states in beam search");

using int64 = std::int64_t;
using uint64 = std::uint64_t;

namespace {

struct State {
  CoreField field;
  int num_2dub;
  int ojama;
  int expect;  // expected score by rensaDetector
  int from;
  Decision first_decision;
};

struct SearchResult {
  Decision decision;
  int num_2dub;
  int ojama;
  int expect;
};

void generateNextStates(const State& state, const Kumipuyo& kumi, int from,
                        std::unordered_set<uint64>& visited, std::vector<State>& states) {
  auto callback = [&state, &from, &visited, &states](const RefPlan& plan) {
    const CoreField field = plan.field();
    RensaResult result = plan.rensaResult();

    uint64 h = field.hash();
    if (!visited.insert(h).second)
      return;

    if (plan.isRensaPlan()) {
      if (result.chains > 2)
        return;
      if (result.score < 680)
        return;

      int ojama = result.score / 70;
      State next = State {
        .field = field,
        .num_2dub = state.num_2dub + 1,
        .ojama = ojama,
        .expect = 0,
        .from = from,
        .first_decision = (from < 0) ? plan.decision(0) : state.first_decision
      };
      states.push_back(next);

      return;
    }

    // Expected number of Ojama puyos to send in future.
    int expect = 0;
    auto detect_callback = [&expect](CoreField&& f, const ColumnPuyoList&) {
      RensaResult r = f.simulate();
      expect = std::max(expect, r.score / 40);
    };
    bool prohibits[FieldConstant::MAP_WIDTH] {};
    RensaDetector::detectByDropStrategy(field, prohibits,
                                        PurposeForFindingRensa::FOR_FIRE, 2, 13,
                                        detect_callback);

    State next = State {
      .field = field,
      .num_2dub = state.num_2dub,
      .ojama = 0,
      .expect = expect,
      .from = from,
      .first_decision = (from < 0) ? plan.decision(0) : state.first_decision
    };
    states.push_back(next);
  };
  Plan::iterateAvailablePlans(state.field, {kumi}, 1, callback);
}

SearchResult search(CoreField field, const KumipuyoSeq& vseq, int search_turns) {
  CHECK_GE(vseq.size(), search_turns);

  std::vector<std::vector<State>> q_states(search_turns + 1);

  const State init_state = State {
    .field = field,
    .num_2dub = 0,
    .ojama = 0,
    .from = -1,
    .first_decision = Decision(0, 0)
  };
  q_states[0].push_back(init_state);
  for (int t = 0; t < search_turns; ++t) {
    std::unordered_set<uint64> visited;
    const auto& que = q_states[t];
    std::vector<State>& next_states = q_states[t + 1];
    for (size_t i = 0; i < que.size(); ++i)
      generateNextStates(que[i], vseq.get(t), ((t == 0) ? -1 : i), visited, next_states);

    if (next_states.empty())
      break;
    std::sort(next_states.begin(), next_states.end(),
              [](const State& a, const State& b) {
                if (a.num_2dub == b.num_2dub) {
                  if (a.ojama == b.ojama)
                    return a.expect > b.expect;
                  return a.ojama > b.ojama;
                }
                return a.num_2dub > b.num_2dub;
              });
    if (static_cast<int>(next_states.size()) >= FLAGS_beam_width)
      next_states.erase(next_states.begin() + FLAGS_beam_width, next_states.end());

    const State best = *next_states.begin();
    if (std::all_of(next_states.begin(), next_states.end(),
        [&best](const State& s){ return s.first_decision == best.first_decision; })) {
      search_turns = t + 1;
      break;
    }
  }

  SearchResult result = SearchResult {
    .decision = Decision(0, 0),
    .num_2dub = -1,
    .ojama = -1,
    .expect = -1
  };
  for (int t = 0; t < search_turns; ++t) {
    for (const auto& s : q_states[t]) {
      if (s.num_2dub > result.num_2dub || (s.num_2dub == result.num_2dub && s.ojama > result.ojama)) {
        result.decision = s.first_decision;
        result.num_2dub = s.num_2dub;
        result.ojama = s.ojama;
        result.expect = s.expect;
      }
    }
  }

  return result;
}

}

class Quick2DubAI : public AI {
 public:
  Quick2DubAI(int argc, char* argv[]) : AI(argc, argv, "quick") {}
  virtual ~Quick2DubAI() {}
  
  virtual DropDecision think(int frame_id, const CoreField& field, const KumipuyoSeq& seq,
                             const PlayerState&, const PlayerState&, bool fast) const {
    UNUSED_VARIABLE(fast);

    int num_simulate = 0;
    int64 start_time = currentTimeInMillis();
    int64 now = start_time;
    std::vector<int> ojamas[7][4];  // [1<=x<=6][0<=r<4]
    for (num_simulate = 0; num_simulate < FLAGS_max_simulate; now = currentTimeInMillis()) {
      if (now >= start_time + FLAGS_time_limit)
        break;

      int search_turns = FLAGS_search_turns;
      KumipuyoSeq vseq = seq;  // 'v' stands for "virtual"
      vseq.append(KumipuyoSeqGenerator::generateRandomSequenceWithSeed(search_turns, frame_id + num_simulate));
      SearchResult result = search(field, vseq, search_turns);
      ++num_simulate;
      if (result.ojama < 0)
        continue;

      const Decision& decision = result.decision;
      auto& list = ojamas[decision.axisX()][decision.rot()];
      list.push_back(result.ojama);
      if (static_cast<int>(list.size()) >= (FLAGS_max_simulate + 1) / 2)
        break;
    }

    double best_expect = 0;
    Decision best_decision(3, 0);
    for (int x = 1; x <= 6; ++x) {
      for (int r = 0; r < 4; ++r) {
        if (ojamas[x][r].empty())
          continue;
        const auto& list = ojamas[x][r];
        double sum = std::accumulate(list.begin(), list.end(), 0);
        double expect = sum / list.size();
        if (best_expect < expect) {
          best_expect = expect;
          best_decision.x = x;
          best_decision.r = r;
        }
      }
    }

    int64 end_time = currentTimeInMillis();

    // TODO: make a message to display
    std::ostringstream oss;
    oss << "Time:" << (end_time - start_time) << "[ms]_/_"
        << "Simulates:" << num_simulate << "_/_"
        << "ExpectOjama:" << best_expect;
    return DropDecision(best_decision, oss.str());
  }
};

int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  Quick2DubAI(argc, argv).runLoop();

  return 0;
}
