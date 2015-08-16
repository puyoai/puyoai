#include "cpu/peria/ai.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <gflags/gflags.h>

#include "core/kumipuyo_seq.h"
#include "core/kumipuyo_seq_generator.h"
#include "solver/endless.h"

DEFINE_string(type, "peria", "Choose one from \"peria\", \"2dub\", and \"full\"");
DEFINE_int32(loop, 5, "The number of playouts");
DEFINE_int32(num_hands, 50, "The number of TSUMOs to play in one game.");
DEFINE_bool(verbose, false, "Display all hands");


int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  std::unique_ptr<AI> ai(new peria::Ai(argc, argv));

  Endless worker(std::move(ai));
  worker.setVerbose(FLAGS_verbose);

  std::vector<int> rensa_count(20, 0);
  int score = 0;
  for (int i = 0; i < FLAGS_loop; ++i) {
    KumipuyoSeq seq = KumipuyoSeqGenerator::generateACPuyo2Sequence();
    seq = seq.subsequence(0, FLAGS_num_hands);
    EndlessResult result = worker.run(seq);
    ++rensa_count[result.maxRensa];
    score += result.score;
    std::cout << "Test : " << (i + 1) << " / " << FLAGS_loop << "\n";
  }
  score /= FLAGS_loop;


  std::cout << "Rensa:\n";
  for (int r = 0; r < 20; ++r) {
    std::cout << std::setw(2) << r << " : " << std::setw(4) << rensa_count[r] << "\n";
  }
  std::cout << "Score: " << score << "\n";
  
  return 0;
}
