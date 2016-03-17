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
#include "solver/puyop.h"

using Decisions = std::vector<Decision>;

class Nazopuyo {
 public:
  Nazopuyo(const toml::Value& v) {
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

  std::vector<Decisions> Solve() {
    std::vector<Decisions> solutions;

    auto checkAnswer = [&](const RefPlan& plan) {
      if (!plan.isRensaPlan())
        return;

      if (plan.chains() < chain_)
        return;
      if (clear_ && !plan.field().isZenkeshi())
        return;
      // TODO: add check of other requirements.

      // Solved
      solutions.push_back(plan.decisions());
    };

    // TODO: iterating one by one, so we can support vanishing in middle of the solution.
    // TODO: Prune unnecessary tries.
    // TODO: Parallelize iteration.
    Plan::iterateAvailablePlans(field_, seq_, seq_.size(), checkAnswer);

    return solutions;
  }

  const CoreField& field() const { return field_; }
  const KumipuyoSeq& seq() const { return seq_; }

 private:
  CoreField field_;
  KumipuyoSeq seq_;

  // Requirements to solve
  int chain_ = 0;  // rensa
  bool clear_ = false;  // zenkeshi
  int colors_ = 0;  // # of colors to be vanished at the same time
  int puyos_ = 0; // # of puyos to be vanished at the same time
};

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
    std::vector<Decisions> solutions = nazo.Solve();

    if (solutions.size()) {
      std::cout << "Found " << solutions.size() << " solution(s).\n";
      std::cout << "One solution is " << makePuyopURL(nazo.field(), nazo.seq(), solutions.front()) << "\n";

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
