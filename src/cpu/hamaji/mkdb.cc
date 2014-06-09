#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sstream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "db.h"
#include "field.h"

int main(int argc, char* argv[]) {
  ParseCommandLineFlags(&argc, &argv, true);
  InitGoogleLogging(argv[0]);

  FLAGS_logtostderr = true;

  vector<Match> matches;
  vector<Match> finished_matches;
  for (int i = 1; i < argc; i++) {
    parseMatches(argv[i], 20, 0, &matches);
    {
      char fname_buf[9999];
      sprintf(fname_buf, "db/%s", basename(argv[i]));
      FILE* fp = fopen(fname_buf, "wb");
      CHECK(fp) << fname_buf;
      for (size_t j = 0; j < matches.size(); j++) {
        const string& seq = matches[j].seq;
        CHECK_EQ(seq.size(), matches[j].decisions.size() * 3);
        string decisions;
        bool* swapped = new bool[seq.size() / 3];
        string normalized = normalizeSeq(seq, swapped);
        for (size_t k = 0; k < matches[j].decisions.size(); k++) {
          Decision decision = matches[j].decisions[k];
          CHECK(decision.isValid());
          if (swapped[k]) {
            switch (decision.r) {
            case 0:
            case 2:
              decision.r = 2 - decision.r;
              break;
            case 1:
            case 3:
              decision.x += 2 - decision.r;
              decision.r = 4 - decision.r;
              break;
            }
          }
          decisions += decision.x + '0';
          decisions += "URDL"[decision.r];
          decisions += '-';
        }
        delete[] swapped;

        fprintf(fp, "%s %s\n",
                normalized.c_str(), decisions.c_str());
      }
      fclose(fp);
      copy(matches.begin(), matches.end(), back_inserter(finished_matches));
      matches.clear();
    }
  }

  matches.swap(finished_matches);

  size_t num_decisions = 0;
  for (size_t i = 0; i < matches.size(); i++) {
    num_decisions += matches[i].decisions.size();
  }

  LOG(INFO) << "num_matches=" << matches.size()
            << ", num_decisions=" << num_decisions;

  int eof_reason_stat[Match::NUM_EOF_REASON];
  memset(&eof_reason_stat, 0, sizeof(eof_reason_stat));
  for (size_t i = 0; i < matches.size(); i++) {
    eof_reason_stat[matches[i].eof_reason]++;
  }
  ostringstream ss;
  for (int i = 0; i < Match::NUM_EOF_REASON; i++) {
    if (i)
      ss << ' ';
    ss << Match::kEofReasonStrs[i] << '=' << eof_reason_stat[i];
  }
  LOG(INFO) << ss.str();
}
