#include "cpu/peria/pai.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <sstream>
#include <vector>

#include "base/time.h"
#include "core/core_field.h"
#include "core/kumipuyo_seq_generator.h"
#include "core/plan/plan.h"
#include "core/rensa/rensa_detector.h"
#include "core/score.h"

#include "pattern.h"

DEFINE_int32(beam_length, 30, "The number of Kumipuyos to append in simulations.");
DEFINE_int32(beam_width_1, 400, "Bandwidth in the beamseach");
DEFINE_int32(beam_width_2, 100, "Bandwidth in the beamseach");
DEFINE_int32(known_length, 5, "Assume at most this number of Tsumos are known.");

#define USE_TEMPLATE 0

namespace peria {

namespace {

struct State;

using int64 = std::int64_t;
using EvaluateFunc = std::function<void(State*)>;

// TODO: Introduce part of enemy's state to handle their attacks.
struct State {
  Decision firstDecision;
  CoreField field;
  bool isZenkeshi;
  int score;
  int expectChain;
  int expectScore;
  int frameId;

  // infomation about enemy's attack
  int enemyPoint;
  int enemyFrame;

  // Debug info
  int from;

  double key;    // to be used in beam-search sorting
  double value;  // to be used in picking up
};

void GenerateNext(State state, int from, const EvaluateFunc& evalFunc,
                  const RefPlan& plan, std::vector<State>* nextStates) {
  if (!state.firstDecision.isValid()) {
    state.firstDecision = plan.firstDecision();
  }
  state.from = from;
  state.field = plan.field();
  state.score += plan.score();
  if (plan.isRensaPlan()) {
    if (state.isZenkeshi) {
      state.score += ZENKESHI_BONUS;
    }
    state.isZenkeshi = plan.hasZenkeshi();
    state.enemyPoint -= plan.score();
    if (state.enemyPoint <= 0) {
      state.enemyFrame = -1;
    }
  }

  // Simulate enemy's attack
  // TODO: Work for TAIOU
  if (state.enemyFrame > 0 && state.enemyFrame < state.frameId + plan.totalFrames()) {
    int numOjama = state.enemyPoint / SCORE_FOR_OJAMA;
    numOjama = std::min(numOjama, 5 * 6);
    state.frameId += state.field.fallOjama((numOjama + 3) / 6);
    state.enemyPoint -= numOjama * SCORE_FOR_OJAMA;
  }
  state.frameId += plan.totalFrames();

  int expectScore = 0;
  int expectChain = 0;
  bool prohibits[FieldConstant::MAP_WIDTH] {};
  auto complementCallback = [&expectScore, &expectChain](CoreField&& field, const ColumnPuyoList&) {
      RensaResult result = field.simulate();
      expectScore = std::max(expectScore, result.score);
      expectChain = std::max(expectChain, result.chains);
  };
  RensaDetector::detectByDropStrategy(state.field, prohibits, PurposeForFindingRensa::FOR_FIRE, 2, 13, complementCallback);
  state.expectScore = expectScore;
  state.expectChain = expectChain;

  evalFunc(&state);

  nextStates->push_back(state);
}

void evalScore(State* s) {
  s->key = s->expectScore;
  s->value = s->score > 100 ? s->score : 0;
}

#if USE_TEMPLATE
void evalTemplate(State* s) {
  const CoreField& field = s->field;

  int best = 0;
  for (const Pattern& pattern : Pattern::GetAllPattern()) {
    int score = pattern.Match(field);
    if (score > best) {
      best = score;
    }
  }

  s->key = best;
  s->value = best;
}
#endif

bool CompKey(const State& a, const State& b) {
  return a.key > b.key;
};

}  // namespace

Pai::Pai(int argc, char* argv[]): ::AI(argc, argv, "peria") {}

Pai::~Pai() {}

DropDecision Pai::think(int frameId,
                        const CoreField& field,
                        const KumipuyoSeq& seq,
                        const PlayerState& myState,
                        const PlayerState& enemyState,
                        bool fast) const {
  std::ostringstream oss;

  int64 startTime = currentTimeInMillis();
  int64 dueTime = startTime + (fast ? 30 : 300);

  int fullIterationDepth = FLAGS_beam_length;
  auto evaluator = evalScore;

#if USE_TEMPLATE
  if (!myState.hasZenkeshi && field.countPuyos() < 10) {
    fullIterationDepth = FLAGS_beam_length / 2;
    evaluator = evalTemplate;
  }
#endif

  const int detectIterationDepth = std::min(seq.size(), fullIterationDepth);
  const int unknownIterationDepth = fullIterationDepth - detectIterationDepth;
  std::vector<std::vector<State>> states(fullIterationDepth + 1);
  CHECK_GE(unknownIterationDepth, 0);
  LOG(INFO) << detectIterationDepth << " / " << fullIterationDepth << " search";

  Decision finalDecision;
  double bestValue = -1;
  int numSameValue = 0;
  std::mt19937 prg;

  State firstState;
  // intentionally leave firstState.firstDecision unset
  firstState.field = field;
  firstState.isZenkeshi = myState.hasZenkeshi;
  firstState.score = myState.unusedScore;
  firstState.expectChain = 0;
  firstState.expectScore = 0;
  firstState.frameId = frameId;
  firstState.enemyPoint = enemyState.currentRensaResult.score;
  firstState.enemyFrame = enemyState.isRensaOngoing() ? enemyState.rensaFinishingFrameId() : -1;
  // TODO: If the enemy is not firing rensa, make some enemy status with
  // probability based on something.

  states[0].push_back(firstState);
  for (int i = 0; i < detectIterationDepth; ++i) {
    auto& nextStates = states[i + 1];
    nextStates.reserve(FLAGS_beam_width_1);
    std::vector<int> fromCount(FLAGS_beam_width_1, 0);
    int used = 0;
    for (int j = 0; j < static_cast<int>(states[i].size()); ++j) {
      const State& s = states[i][j];
      int f = s.from;
      if (++fromCount[f] > 5)
        continue;
      auto generateNext = std::bind(GenerateNext, s, j, evaluator, std::placeholders::_1, &nextStates);
      Plan::iterateAvailablePlans(field, {seq.get(i)}, 1, generateNext);
      if (++used > FLAGS_beam_width_1)
        break;
    }
    std::sort(nextStates.begin(), nextStates.end(), CompKey);
    for (const State& s : nextStates) {
      if (s.value == bestValue) {
        ++numSameValue;
        if (prg() % numSameValue == 0) {
          finalDecision = s.firstDecision;
          bestValue = s.value;
        }
      }
      if (s.value > bestValue) {
        finalDecision = s.firstDecision;
        bestValue = s.value;
        numSameValue = 1;
      }
    }
  }

  if (states[detectIterationDepth].size() == 0) {
    oss << "Die";
    return DropDecision(Decision(3, 2), oss.str());
  }

  int64 detectTime = currentTimeInMillis();
  oss << "Known: "
      << (detectTime - startTime) << " ms / "
      << states[detectIterationDepth].size() << " states / "
      << detectIterationDepth << " hands / "
      << "(" << finalDecision.axisX() << "-" << finalDecision.rot() << ")"
      << " " << bestValue << " pts\n";

  int nTest = 0;
  std::map<Decision, std::vector<double>> vote;
  do {
    Decision decision;
    double value = 0;

    KumipuyoSeq pseudoSeq = KumipuyoSeqGenerator::generateRandomSequence(unknownIterationDepth);
    for (int i = detectIterationDepth; i < fullIterationDepth; ++i) {
      auto& nextStates = states[i + 1];
      nextStates.clear();
      std::vector<int> fromCount(FLAGS_beam_width_1, 0);
      int used = 0;
      for (int j = 0; j < static_cast<int>(states[i].size()); ++j) {
        const State& s = states[i][j];
        if (++fromCount[j] > 3)
          continue;
        auto generateNext = std::bind(GenerateNext, s, j, evaluator, std::placeholders::_1, &nextStates);
        Plan::iterateAvailablePlans(field, {pseudoSeq.get(i - detectIterationDepth)}, 1, generateNext);
        if (++used > FLAGS_beam_width_2)
          break;
      }
      std::sort(nextStates.begin(), nextStates.end(), CompKey);
      for (const State& s : nextStates) {
        if (s.value > value) {
          decision = s.firstDecision;
          value = s.value;
        }
      }
    }

    if (decision.isValid()) {
      LOG(INFO) << "vote: " << decision << " " << value;
      vote[decision].push_back(value);
    }

    ++nTest;
    int64 guessTime = currentTimeInMillis();
    int64 avgGuessTime = (guessTime - detectTime) / nTest;
    if (guessTime + avgGuessTime >= dueTime)
      break;
  } while (true);

  int64 endTime = currentTimeInMillis();
  oss << "Unknown: " << (endTime - detectTime) << " ms / " << nTest << " tests\n";

  if (vote.size()) {
    for (auto& v : vote) {
      double avg = std::accumulate(v.second.begin(), v.second.end(), 0.0);
      avg /= v.second.size();
      LOG(INFO) << v.first << " " << v.second.size() << " " << avg;
      if (avg > bestValue) {
        bestValue = avg;
        finalDecision = v.first;
      }
    }
  }
  if (vote.size() && vote[finalDecision].size()) {
    oss << "Final: (" << finalDecision.axisX() << "-" << finalDecision.rot() << ")"
        << " " << bestValue << " pts with "
        << vote[finalDecision].size() << " votes\n";
  } else {
    oss << "Final: no updates\n";
  }

#if 0
  // Debug output
  std::cerr << field.toDebugString() << "\n";
  for (auto& ss : states) {
    std::cerr << "> ";
    for (auto& s : ss) {
      std::cerr << s.from << " ";
    }
    std::cerr << "\n";
  }
#endif

  return DropDecision(finalDecision, oss.str());
}

}  // namespace peria
