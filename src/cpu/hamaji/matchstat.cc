#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sstream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "db.h"
#include "field.h"
#include "ratingstats.h"

DEFINE_int32(num_turns, 40, "");

int main(int argc, char* argv[]) {
  ParseCommandLineFlags(&argc, &argv, true);
  InitGoogleLogging(argv[0]);

  FLAGS_logtostderr = true;

  int eof_reason_stat[Match::NUM_EOF_REASON];
  memset(&eof_reason_stat, 0, sizeof(eof_reason_stat));

  RatingStats stats;
  for (int i = 1; i < argc; i++) {
    vector<Match> matches;
    parseMatches(argv[i], FLAGS_num_turns + 2, Match::OJAMA_PUYO_MASK | Match::VANISH_PUYO_MASK, &matches);

    for (size_t j = 0; j < matches.size(); j++) {
      const Match& match = matches[j];

      eof_reason_stat[match.eof_reason]++;
      if (match.eof_reason != Match::MANY_TURNS)
        continue;

      LF f;
      for (int t = 0; t < FLAGS_num_turns; t++) {
        string next;
        for (int k = 0; k < 6; k++) {
          next.push_back(match.seq[t*3+k/2*3+k%2] - 'A' + 4);
        }
        //puts(match.seq.c_str());
        //puts(f.GetDebugOutput(next).c_str());

        vector<LP> plans;
        f.FindAvailablePlans(next, &plans);

        for (size_t k = 0; k < plans.size(); k++) {
          for (const LP* p = &plans[k]; p; p = p->parent) {
            //int score = p->score - (p->parent ? p->parent->score : 0);
            //stats.add_max_score(score, p->parent);

            int chain_cnt =
              p->chain_cnt - (p->parent ? p->parent->chain_cnt : 0);
            stats.add_chain_stats(chain_cnt, stats.total_count, t);
          }
        }

        f.PutDecision(match.decisions[t], next[0], next[1]);
      }

      stats.total_count++;
    }
  }

  ostringstream ss;
  for (int i = 0; i < Match::NUM_EOF_REASON; i++) {
    if (i)
      ss << ' ';
    ss << Match::kEofReasonStrs[i] << '=' << eof_reason_stat[i];
  }
  LOG(INFO) << ss.str();

  printf("num_matches=%d\n", stats.total_count);
  stats.Print();
}
