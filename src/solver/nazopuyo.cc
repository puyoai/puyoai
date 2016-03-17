#include <gflags/gflags.h>
#include <glog/logging.h>
#include <toml/toml.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "core/core_field.h"
#include "core/decision.h"
#include "core/kumipuyo_seq.h"
#include "core/plan/plan.h"
#include "core/puyo_controller.h"
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

class Nazopuyo {
 public:
  Nazopuyo(const toml::Value& v);

  bool Solve();

  const CoreField& field() const { return field_; }
  const KumipuyoSeq& seq() const { return seq_; }
  const std::vector<Decisions>& solutions() const { return solutions_; }

 private:
  bool verifySolution(const CoreField&, const RensaResult&);
  bool iterate(const CoreField&);
  bool iterate(const CoreField&, Decisions&, int, int);
  
  CoreField field_;
  KumipuyoSeq seq_;

  std::vector<Decisions> solutions_;

  // Requirements to solve
  int chain_ = 0;  // rensa
  bool clear_ = false;  // zenkeshi
  int colors_ = 0;  // # of colors to be vanished at the same time
  int puyos_ = 0; // # of puyos to be vanished at the same time
  bool unique_ = false; // set true if you need one solution.
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
  // TODO: Prune unnecessary tries.
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

bool Nazopuyo::iterate(const CoreField& field) {
  solutions_.clear();
  Decisions decisions;
  return iterate(field, decisions, 0, seq_.size());
}

bool Nazopuyo::iterate(const CoreField& field, Decisions& decisions,
                       int currentDepth, int maxDepth) {
  const Kumipuyo& kumipuyo = seq_.get(currentDepth);

  bool found = false;
  const int num_decisions = (kumipuyo.axis == kumipuyo.child) ? 11 : 22;
  for (int j = 0; j < num_decisions; j++) {
    const Decision& decision = DECISIONS[j];
    if (!PuyoController::isReachable(field, decision))
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
    }

    if (!nextField.isEmpty(3, 12)) {
      decisions.pop_back();
      continue;
    }

    // TODO: continue if nextField and remained sequences cannot serve requirements.

    if (currentDepth + 1 < maxDepth) {
      bool f = iterate(nextField, decisions, currentDepth + 1, maxDepth);
      if (f && unique_) {
        return true;
      }
      found = found || f;
    }
    decisions.pop_back();
  }
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
    // TODO: measure time
    nazo.Solve();
    std::vector<Decisions> solutions = nazo.solutions();

    if (solutions.size()) {
      std::cout << "Found " << solutions.size() << " solution(s).\n"
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
