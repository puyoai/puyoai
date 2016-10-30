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

// TODO: Rename class not to be algorithm specific name.
class BeamSearch {
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

  static bool shouldHonki(const PlayerState& enemy);

  static constexpr int kSearchDepth = 30;
  static constexpr int kMaxSearchWidth = 440;

  // |frames| figures frame dration by the enemy's rensa will finish.
  BeamSearch(const PlayerState& me, const KumipuyoSeq& seq, int frames);
  Decision run(int* t = nullptr);

 protected:
  void init();
  void expand(const SearchState& state, const int index);

  int best_score_ = 0;
  Decision best_decision_;
  std::vector<std::deque<SearchState>> beam_;
  std::vector<std::unordered_set<std::uint64_t>> visited_;
  KumipuyoSeq seq_;

  const double time_limit_;
};

}  // namespace peria
