#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "ai.h"

#include "core/sequence_generator.h"
#include "solver/endless.h"

constexpr int TEST_TIMES = 500;
constexpr double NORMSINV = 1.959963986; // For 0.025 one-side.

int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  Endless endless(std::move(std::unique_ptr<AI>(new munetoshi::AI(argc, argv))));

  std::array<int, TEST_TIMES> score_array;
  std::array<int, TEST_TIMES> chain_array;

  int64_t total_score = 0;
  int total_chain = 0;

  int main_count = 0;
  int death_count = 0;
  int runout_count = 0;

  for (int i = 0; i < TEST_TIMES; ++i) {
      KumipuyoSeq seq = generateRandomSequenceWithSeed(i);
      EndlessResult result = endless.run(seq);

      std::cout << i << ": " << "type = " << static_cast<int>(result.type) << ", score = " << result.score << ", chain = " << result.maxRensa << std::endl;
      switch (result.type) {
      case EndlessResult::Type::MAIN_CHAIN:
          chain_array[main_count] = result.maxRensa;
          score_array[main_count] = result.score;

          total_score += result.score;
          total_chain += result.maxRensa;
          ++main_count;
          break;
      case EndlessResult::Type::DEAD:
          ++death_count;
          break;
      case EndlessResult::Type::PUYOSEQ_RUNOUT:
          ++runout_count;
          break;
      case EndlessResult::Type::ZENKESHI:
          break;
      }
  }

  if (main_count < 2) {
      return 0;
  }

  double ave_score = total_score / ((double) main_count);
  double ave_chain = total_chain / ((double) main_count);
  double score_sum_sq_deviation = 0;
  double chain_sum_sq_deviation = 0;
  for (int i = 0; i < main_count; ++i) {
      score_sum_sq_deviation += std::pow(score_array[i] - ave_score, 2);
      chain_sum_sq_deviation += std::pow(chain_array[i] - ave_chain, 2);
  }
  double score_unbiased_var = score_sum_sq_deviation / (main_count - 1);
  double chain_unbiased_var = chain_sum_sq_deviation / (main_count - 1);

  double score_confidence_delta = std::pow(score_unbiased_var / main_count, 0.5) * NORMSINV;
  double chain_confidence_delta = std::pow(chain_unbiased_var / main_count, 0.5) * NORMSINV;

  std::cout << std::endl;
  std::cout << "ave score = " << ave_score << ", norm confidence (p=0.95) ["
          << ave_score - score_confidence_delta << ", "
          << ave_score + score_confidence_delta  << "]" << std::endl;
  std::cout << "ave chain = " << ave_chain << ", norm confidence (p=0.95) ["
          << ave_chain - chain_confidence_delta << ", "
          << ave_chain + chain_confidence_delta  << "]" << std::endl;
  std::cout << "death ratio = " << ((float) death_count) / TEST_TIMES << std::endl;
  std::cout << "runout ratio = "  << ((float) runout_count) / TEST_TIMES << std::endl;
  return 0;
}
