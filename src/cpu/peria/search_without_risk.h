#pragma once

#include <cstdint>
#include <deque>
#include <unordered_set>
#include <vector>

#include "core/core_field.h"
#include "core/decision.h"
#include "core/kumipuyo_seq.h"

class Decision;
struct PlayerState;

namespace peria {

class SearchWithoutRisk {
 public:
  struct SearchState {
    bool compareTo(const SearchState& other) const;

    CoreField field;
    bool hasZenkeshi = false;
    int frame = 0;  // how many frames by enemy's rensa will finish.
    int score = 0;
    int expect = 0;
    Decision decision;
  };

  static bool shouldRun(const PlayerState& enemy);

  static constexpr int kSearchDepth = 30;
  static constexpr int kMaxSearchWidth = 440;

  // |frames| figures frame dration by the enemy's rensa will finish.
  SearchWithoutRisk(const PlayerState& me, const KumipuyoSeq& seq, int frames);
  Decision run();
  Decision bestDecision() const;

  int countBeam() const;

 protected:
  void init();
  void expand(const SearchState& state, const int index);

  int best_score_ = 0;
  Decision best_decision_;
  std::vector<std::deque<SearchState>> states_;
  std::vector<std::unordered_set<std::uint64_t>> visited_;
  KumipuyoSeq seq_;

  std::array<std::array<double, 4>, 7> total_score_ {};
  std::array<std::array<int, 4>, 7> total_score_count_ {};

  const double time_limit_;
};

}  // namespace peria
