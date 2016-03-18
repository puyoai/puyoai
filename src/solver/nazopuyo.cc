#include <gflags/gflags.h>
#include <glog/logging.h>
#include <toml/toml.h>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "base/time.h"
#include "core/core_field.h"
#include "core/decision.h"
#include "core/kumipuyo_seq.h"
#include "core/plan/plan.h"
#include "core/puyo_color.h"
#include "solver/puyop.h"

using Decisions = std::vector<Decision>;

namespace {

const Decision DECISIONS[] = {
  Decision(2, 3), Decision(3, 3), Decision(3, 1), Decision(4, 1),
  Decision(5, 1), Decision(1, 2), Decision(2, 2), Decision(3, 2),
  Decision(4, 2), Decision(5, 2), Decision(6, 2), Decision(1, 1),
  Decision(2, 1), Decision(4, 3), Decision(5, 3), Decision(6, 3),
  Decision(1, 0), Decision(2, 0), Decision(3, 0), Decision(4, 0),
  Decision(5, 0), Decision(6, 0),
};

inline bool isDecisionAvailable(const CoreField& field, const Decision& decision) {
#define E(x) (field.height(x) <= 11)
#define F(x) (field.height(x) <= 12)
  switch (decision.r) {
  case 0:
    switch (decision.x) {
    case 1: return E(3) && F(2) && F(1);
    case 2: return E(3) && F(2);
    case 3: return E(3);
    case 4: return E(3) && F(4);
    case 5: return E(3) && F(4) && F(5);
    case 6: return E(3) && F(4) && F(5) && F(6);
    }

  case 1:
    switch (decision.x) {
    case 1: return E(3) && F(2) && F(1);
    case 2: return E(3) && F(2);
    case 3: return E(3) && F(4);
    case 4: return E(3) && F(4) && F(5);
    case 5: return E(3) && F(4) && F(5) && F(6);
    }

  case 2:
    switch (decision.x) {
    case 1: return E(3) && F(2) && E(1);
    case 2: return E(3) && E(2);
    case 3: return E(3);
    case 4: return E(3) && E(4);
    case 5: return E(3) && F(4) && E(5);
    case 6: return E(3) && F(4) && F(5) && E(6);
    }

  case 3:
    switch (decision.x) {
    case 2: return E(3) && F(2) && F(1);
    case 3: return E(3) && F(2);
    case 4: return E(3) && F(4);
    case 5: return E(3) && F(4) && F(5);
    case 6: return E(3) && F(4) && F(5) && F(6);
    }
  }
#undef E
#undef F

  CHECK(false);
  return false;
}

class Nazopuyo {
 public:
  Nazopuyo(const toml::Value& v);

  bool Solve();

  const CoreField& field() const { return field_; }
  const KumipuyoSeq& seq() const { return seq_; }
  const std::vector<Decisions>& solutions() const { return solutions_; }

 private:
  bool verifySolution(const CoreField&, const RensaResult&);
  bool possibleToSolve(const CoreField&);
  bool iterate(const CoreField&);
  bool iterate(const CoreField&, Decisions&, int, int);

  int remainPuyos(PuyoColor c) { return remainPuyos_[ordinal(c)]; }
  
  CoreField field_;
  KumipuyoSeq seq_;

  std::vector<Decisions> solutions_;

  // Requirements to solve
  int chain_ = 0;  // rensa
  bool clear_ = false;  // zenkeshi
  int colors_ = 0;  // # of colors to be vanished at the same time
  int puyos_ = 0; // # of puyos to be vanished at the same time
  bool unique_ = false; // set true if you need one solution.

  int remainPuyos_[NUM_PUYO_COLORS] {};
};

Nazopuyo::Nazopuyo(const toml::Value& v) {
  std::string field_str;
  const toml::Value* field = v.find("field");
  CHECK(field);
  for (auto& line : field->as<toml::Array>()) {
    field_str += line.as<std::string>();
  }
  field_ = CoreField(field_str);

  const toml::Value* seq = v.find("seq");
  CHECK(seq);
  std::string kumipuyo_str;
  for (auto& kumi : seq->as<toml::Array>()) {
    std::string puyos = kumi.as<std::string>();
    CHECK_EQ(2U, puyos.size());
    kumipuyo_str += puyos;
  }
  seq_ = KumipuyoSeq(kumipuyo_str);

  // Parse requirements.
  const toml::Value* chain = v.find("chain");
  if (chain && chain->is<int>())
    chain_ = chain->as<int>();
  
  const toml::Value* clear = v.find("clear");
  if (clear && clear->is<bool>())
    clear_ = clear->as<bool>();

  const toml::Value* colors = v.find("colors");
  if (colors && colors->is<int>())
    colors_ = colors->as<bool>();
    
  const toml::Value* puyos = v.find("puyos");
  if (puyos && puyos->is<bool>())
    puyos_ = puyos->as<bool>();
}

bool Nazopuyo::Solve() {
  for (PuyoColor c : NORMAL_PUYO_COLORS) {
    remainPuyos_[ordinal(c)] = field_.countColor(c);
  }
  for (int i = 0; i < seq_.size(); ++i) {
    const Kumipuyo& kp = seq_.get(i);
    remainPuyos_[ordinal(kp.axis)]++;
    remainPuyos_[ordinal(kp.child)]++;
  }

  // TODO: Parallelize iteration.
  iterate(field_);
  return solutions_.size();
}

bool Nazopuyo::verifySolution(const CoreField& field, const RensaResult& result) {
  if (result.chains < chain_)
    return false;
  if (clear_ && !field.isZenkeshi())
    return false;
  // TODO: add check of other requirements.
  
  // Solved
  return true;
}

bool Nazopuyo::possibleToSolve(const CoreField& field) {
  if (!field.isEmpty(3, 12))
    return false;

  if (chain_) {
    int possibleChain = 0;
    for (PuyoColor c : NORMAL_PUYO_COLORS) {
      int puyos = field.countColor(c) + remainPuyos(c);
      possibleChain += puyos / PUYO_ERASE_NUM;
    }
    if (possibleChain < chain_)
      return false;
  }

  if (clear_) {
    for (PuyoColor c : NORMAL_PUYO_COLORS) {
      int puyos = field.countColor(c) + remainPuyos(c);
      if (puyos < PUYO_ERASE_NUM)
        return false;
    }
  }
  
  return true;
}

bool Nazopuyo::iterate(const CoreField& field) {
  solutions_.clear();
  Decisions decisions;
  return iterate(field, decisions, 0, seq_.size());
}

bool Nazopuyo::iterate(const CoreField& field, Decisions& decisions,
                       int currentDepth, int maxDepth) {
  const Kumipuyo& kumipuyo = seq_.get(currentDepth);
  remainPuyos_[ordinal(kumipuyo.axis)]--;
  remainPuyos_[ordinal(kumipuyo.child)]--;

  bool found = false;
  const int num_decisions = (kumipuyo.axis == kumipuyo.child) ? 11 : 22;
  for (int j = 0; j < num_decisions; j++) {
    const Decision& decision = DECISIONS[j];

    if (!isDecisionAvailable(field, decision))
      continue;

    CoreField nextField(field);
    if (!nextField.dropKumipuyo(decision, kumipuyo))
      continue;

    decisions.push_back(decision);
    bool shouldFire = nextField.rensaWillOccurWhenLastDecisionIs(decision);
    if (shouldFire) {
      RensaResult result = nextField.simulate();
      if (verifySolution(nextField, result)) {
        solutions_.push_back(decisions);
        if (unique_) {
          return true;
        }
        found = true;
      }

      if (!possibleToSolve(nextField)) {
        decisions.pop_back();
        continue;
      }
    }

    if (currentDepth + 1 < maxDepth) {
      bool f = iterate(nextField, decisions, currentDepth + 1, maxDepth);

      if (f && unique_) {
        return true;
      }
      found = found || f;
    }
    decisions.pop_back();
  }

  remainPuyos_[ordinal(kumipuyo.axis)]++;
  remainPuyos_[ordinal(kumipuyo.child)]++;
  return false;
}

} // namespace

int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
#if !OS_WIN
  google::InstallFailureSignalHandler();
#endif

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " problem.toml\n";
    return 0;
  }

  std::ifstream ifs(argv[1]);
  toml::ParseResult result = toml::parse(ifs);
  if (!result.valid()) {
    LOG(ERROR) << result.errorReason;
    return 0;
  }

  const toml::Value& value = result.value;
  if (!value.is<toml::Table>()) {
    LOG(ERROR) << "Value is not a table";
    return 0;
  }
  const toml::Value* nazos = value.find("nazo");
  if (!nazos || !nazos->is<toml::Array>()) {
    LOG(ERROR) << "No nazo problems are found.";
    return 0;
  }

  int numSolved = 0;
  for (const auto& v : nazos->as<toml::Array>()) {
    Nazopuyo nazo(v);

    int64_t startTime = currentTimeInMillis();
    nazo.Solve();
    int64_t endTime = currentTimeInMillis();

    std::vector<Decisions> solutions = nazo.solutions();
    if (solutions.size()) {
      std::cout << "Found " << solutions.size() << " solution(s) in " << (endTime - startTime) << "ms.\n"
                << "One solution is " << makePuyopURL(nazo.field(), nazo.seq(), solutions.front()) << std::endl;

      LOG(INFO) << "Found " << solutions.size() << " solution(s).";
      for (auto& s : solutions) {
        LOG(INFO) << makePuyopURL(nazo.field(), nazo.seq(), s);
      }

      ++numSolved;
    } else {
      LOG(ERROR) << "No solution for:\n"
                 << nazo.field().toDebugString();
    }
  }
  std::cout << numSolved << " / " << nazos->size() << " are solved.\n";
  LOG(INFO) << numSolved << " / " << nazos->size() << " are solved.";
  
  return 0;
}
