#include "search_without_risk.h"

#include <vector>
#include <deque>

#include <glog/logging.h>

#include "base/base.h"
#include "base/time.h"
#include "core/decision.h"
#include "core/kumipuyo_seq.h"
#include "core/kumipuyo_seq_generator.h"
#include "core/plan/plan.h"
#include "core/player_state.h"
#include "core/rensa/rensa_detector.h"

namespace peria {

bool SearchWithoutRisk::SearchState::compareTo(const SearchState& other) const {
  if (expect == other.expect) {
    if (score == other.score) {
      return frame > other.frame;  // earlier is better
    }
    return score < other.score;
  }
  return expect < other.expect;
}

bool SearchWithoutRisk::shouldRun(const PlayerState& enemy) {
  if (!enemy.isRensaOngoing())
    return false;
  CoreField field = enemy.fieldWhenGrounded;
  int puyo_num_from = field.countPuyos();
  field.simulate();
  int puyo_num_used = puyo_num_from - field.countPuyos();
  LOG(INFO) << "Enemy is firing with " << puyo_num_used << " in " << puyo_num_from;

  return puyo_num_used * 2 > puyo_num_from;
}

SearchWithoutRisk::SearchWithoutRisk(const PlayerState& me, const KumipuyoSeq& seq, int frames) :
    best_score_(0),
    states_(kSearchDepth), visited_(kSearchDepth),
    time_limit_(currentTime() + 0.250) {
  // Register starting state.
  SearchState first_state;
  first_state.field = me.field;
  first_state.hasZenkeshi = me.hasZenkeshi;
  first_state.frame = frames;
  first_state.score = 0;
  first_state.decision = Decision();

  // Expand 2 contorls with known Kumipuyos.
  seq_ = seq;
  expand(first_state, 0);
  for (const SearchState& state : states_[1]) {
    expand(state, 1);
  }
}

void SearchWithoutRisk::init() {
  if (best_decision_.isValid()) {
    int x = best_decision_.x;
    int r = best_decision_.r;
    total_score_[x][r] += best_score_;
    ++total_score_count_[x][r];

    best_score_ = 0;
    best_decision_ = Decision();
  }

  // in |sequence|, [0] and [1] are not used.
  seq_ = KumipuyoSeqGenerator::generateRandomSequence(kSearchDepth);
  for (int i = 3; i < kSearchDepth; ++i) {
    states_[i].clear();
    visited_[i].clear();
  }
  for (const SearchState& state : states_[2]) {
    expand(state, 2);
  }
}

Decision SearchWithoutRisk::run() {
  // Dynamic beam search from 3rd states.
  for (int t = 0; currentTime() < time_limit_; ++t) {
    // Reset every |kMaxSearchWidth| times.
    if (t % kMaxSearchWidth == 0) {
      init();
    }

    for (int i = 3; i < kSearchDepth; ++i) {
      std::deque<SearchState>& states = states_[i];
      if (states.empty()) {
        continue;
      }

      std::sort(states.begin(), states.end(),
                [](const SearchState& a, const SearchState& b) { return a.compareTo(b); });
      if (states.size() > kMaxSearchWidth)
        states.resize(kMaxSearchWidth);

      expand(states.front(), i);
      states.pop_front();
    }
  }

  return bestDecision();
}

Decision SearchWithoutRisk::bestDecision() const {
  Decision decision;
  double score = 0;
  for (int x = 1; x <= 6; ++x) {
    for (int r = 0; r < 4; ++r) {
      if (total_score_count_[x][r] == 0)
        continue;
      double avg = total_score_[x][r] / total_score_count_[x][r];
      if (avg > score) {
        decision = Decision(x, r);
        score = avg;
      }
    }
  }
  return decision;
}

int SearchWithoutRisk::countBeam() const {
  int sum = 0;
  for (auto counts : total_score_count_) {
    for (auto c : counts) {
      sum += c;
    }
  }
  return sum;
}

void SearchWithoutRisk::expand(const SearchState& from, const int index) {
  SearchWithoutRisk* self = this;
  auto callback = [&self, &from, &index](const RefPlan& plan)
  {
    const CoreField field = plan.field();
    if (!self->visited_[index + 1].insert(field.hash()).second)
      return;
    int frame = from.frame - plan.totalFrames();
    SearchState next;
    next.field = field;
    next.frame = frame;
    next.decision = from.decision.isValid() ? from.decision : plan.firstDecision();
    if (plan.isRensaPlan()) {
      next.score = from.score + plan.score() + (from.hasZenkeshi ? ZENKESHI_BONUS : 0);
      next.hasZenkeshi = field.isZenkeshi();
      if (next.score > self->best_score_) {
        self->best_score_ = next.score;
        self->best_decision_ = next.decision;
      }
    } else {
      next.score = from.score;
      next.hasZenkeshi = from.hasZenkeshi;
    }

    if (frame < 0)
      return;

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

    next.expect = expect + (next.hasZenkeshi ? ZENKESHI_BONUS : 0);
    if (index < kSearchDepth - 1) {
      self->states_[index + 1].push_back(next);
    }
  };
  Plan::iterateAvailablePlans(from.field, {seq_.get(index)}, 1, callback);
}

}  // namespace peria
