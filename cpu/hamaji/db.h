#ifndef DB_H_
#define DB_H_

#include <string>
#include <vector>

#include <glog/logging.h>

#include "field.h"

struct Match {
  enum EofReason {
    NONE,
    END_MATCH, BROKEN_PUYO, VANISH_PUYO, VANISH_PUYO2,
    OJAMA_PUYO, INCONSISTENT, MANY_TURNS,
    NUM_EOF_REASON,
  };
  enum EofReasonMask {
    VANISH_PUYO_MASK = 1 << VANISH_PUYO,
    OJAMA_PUYO_MASK = 1 << OJAMA_PUYO,
  };
  static const char* kEofReasonStrs[];

  Match();

  string seq;
  vector<Decision> decisions;
  EofReason eof_reason;
};

string normalizeSeq(const string& seq, bool* swapped = 0);
string normalizeSeqUni(const string& seq);

void parseMatches(const char* filename,
                  int num_turns,
                  int noeof_reason_mask,
                  vector<Match>* matches);

#endif  // DB_H_
