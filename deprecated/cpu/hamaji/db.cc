#include "db.h"

#include <stdio.h>
#include <string.h>

#include <algorithm>

#include <glog/logging.h>

static int countTrailingZero(int n) {
  for (int i = 0; i < 4; i++) {
    if (n & (1 << i))
      return i;
  }
  LOG(FATAL) << n;
  return 0;
}

static void countTrailingZeroTwice(int n, int* o) {
  int c = 0;
  for (int i = 0; i < 4; i++) {
    if (n & (1 << i)) {
      o[c] = i;
      c++;
      if (c == 2)
        return;
    }
  }
  LOG(FATAL) << n;
}

static void claimLowest(int a, int* tbl, int* cand) {
  int x = countTrailingZero(cand[a]);
  VLOG(2) << "claimLowest: " << a << " " << x;
  tbl[a] = x;
  for (int i = 0; i < 4; i++) {
    if (a == i) {
      cand[i] = 1 << x;
    } else {
      cand[i] &= ~(1 << x);
    }
  }
  VLOG(2) << "cand: " << cand[0] << ' ' << cand[1]
          << ' ' << cand[2] << ' ' << cand[3];
}

string normalizeSeqUni(const string& seq) {
  VLOG(2) << "normalizeSeq: " << seq;
  int cand[4];
  int tbl[4];
  int cnt[4];
  for (int i = 0; i < 4; i++) {
    cand[i] = 15;
    tbl[i] = -1;
    cnt[i] = 0;
  }

  for (size_t i = 0; i < seq.size(); i += 3) {
    VLOG(2) << i;
    VLOG(2) << "cand: " << cand[0] << ' ' << cand[1]
            << ' ' << cand[2] << ' ' << cand[3];
    VLOG(2) << "tbl: " << tbl[0] << ' ' << tbl[1]
            << ' ' << tbl[2] << ' ' << tbl[3];

    int a = seq[i] - 'A';
    int b = seq[i+1] - 'A';
    if (a == b) {
      cnt[a] += 2;
      claimLowest(a, tbl, cand);
    } else {
      cnt[a]++;
      cnt[b]++;
      if (cnt[a] == cnt[b]) {
        int x = countTrailingZero(cand[a]);
        int y = countTrailingZero(cand[b]);
        if (x != y) {
          claimLowest(a, tbl, cand);
          claimLowest(b, tbl, cand);
        } else {
          int zx[2], zy[2];
          countTrailingZeroTwice(cand[a], zx);
          countTrailingZeroTwice(cand[b], zy);
          CHECK_EQ(zx[0], zy[0]);
          CHECK_EQ(zx[1], zy[1]);
          for (int j = 0; j < 4; j++) {
            if (j == a || j == b) {
              cand[j] = (1 << zx[0]) | (1 << zx[1]);
            } else {
              cand[j] &= ~((1 << zx[0]) | (1 << zx[1]));
            }
          }
        }
      } else if (cnt[a] > cnt[b]) {
        claimLowest(a, tbl, cand);
        claimLowest(b, tbl, cand);
      } else {
        claimLowest(b, tbl, cand);
        claimLowest(a, tbl, cand);
      }
    }

    bool done = true;
    for (int j = 0; j < 4; j++) {
      if (tbl[j] < 0) {
        done = false;
        break;
      }
    }
    if (done)
      break;
  }

  for (int i = 0; i < 4; i++) {
    claimLowest(i, tbl, cand);
  }

  string out;
  for (size_t i = 0; i < seq.size(); i += 3) {
    char a = tbl[seq[i] - 'A'] + 'A';
    char b = tbl[seq[i+1] - 'A'] + 'A';
    if (b < a) {
      out.push_back(b);
      out.push_back(a);
    } else {
      out.push_back(a);
      out.push_back(b);
    }

    out.push_back('-');
  }
  return out;
}

string normalizeSeq(const string& seq, bool* swapped) {
  if (swapped) {
    for (size_t i = 0; i < seq.size() / 3; i++) {
      swapped[i] = false;
    }
  }

  char tbl[4];
  for (int i = 0; i < 4; i++) {
    tbl[i] = 0;
  }

  char cur = 'A';
  for (size_t i = 0; i < seq.size() && cur <= 'D'; i += 3) {
    VLOG(2) << i << " tbl: " << tbl[0] << ' ' << tbl[1]
            << ' ' << tbl[2] << ' ' << tbl[3];
    int a = seq[i] - 'A';
    int b = seq[i+1] - 'A';
    if (a == b) {
      if (!tbl[a]) {
        tbl[a] = cur++;
      }
    } else {
      if (tbl[a]) {
        if (!tbl[b]) {
          tbl[b] = cur++;
        }
      } else if (tbl[b]) {
        tbl[a] = cur++;
      } else {
        if (a > b)
          swap(a, b);

        bool a_first = true;
        for (size_t j = i + 3; j < seq.size(); j += 3) {
          int x = seq[j] - 'A';
          int y = seq[j+1] - 'A';
          if (x > y)
            swap(x, y);

          if (a == x && b == y)
            continue;

          if (a == x || a == y) {
            break;
          } else if (b == x || b == y) {
            a_first = false;
            break;
          }
        }

        if (a_first) {
          tbl[a] = cur++;
          tbl[b] = cur++;
        } else {
          tbl[b] = cur++;
          tbl[a] = cur++;
        }
      }
    }
  }

  string out;
  for (size_t i = 0; i < seq.size(); i += 3) {
    char a = tbl[seq[i] - 'A'];
    char b = tbl[seq[i+1] - 'A'];
    CHECK_NE(a, '\0') << seq;
    CHECK_NE(b, '\0') << seq;
    if (b < a) {
      if (swapped)
        swapped[i/3] = true;
      out.push_back(b);
      out.push_back(a);
    } else {
      out.push_back(a);
      out.push_back(b);
    }

    out.push_back('-');
  }
  return out;
}

static void parseField(char* p, LF* field) {
#if OLD_PARSE_MOVIE
  for (int y = 12; y >= 1; y--) {
    for (int x = 1; x <= 6; x++) {
      int c = *p - '0';
      CHECK_GE(c, 0) << x << "," << y;
      CHECK_LE(c, 5) << x << "," << y;
      field->Set(x, y, "045671"[c] - '0');
      p++;
    }
  }
#else
  *field = LF(p);
#endif
}

static void parseNext(const char* p, string* next) {
#if OLD_PARSE_MOVIE
  next->resize(6);
  for (int i = 0; i < 6; i++) {
    int c = p[i] - '0';
    CHECK_GE(c, 0) << i;
    CHECK_LE(c, 5) << i;
    (*next)[i] = "045671"[c] - '0';
  }
#else
  next->resize(6);
  for (int i = 0; i < 6; i++) {
    if (p[i])
      (*next)[i] = p[i] - '0';
  }
#endif
}

const char* Match::kEofReasonStrs[] = {
  "???",
  "END_MATCH",
  "BROKEN_PUYO",
  "VANISH_PUYO",
  "VANISH_PUYO2",
  "OJAMA_PUYO",
  "INCONSISTENT",
  "MANY_TURNS",
};

Match::Match() : eof_reason(NONE) {}

void parseMatches(const char* filename,
                  int num_turns,
                  int noeof_reason_mask,
                  vector<Match>* matches) {
  Match match;

  LOG(INFO) << "reading " << filename;
  FILE* fp = fopen(filename, "rb");
  CHECK(fp) << filename;
  char buf[1024], pbuf[256], nbuf[6], abuf[256];
  int turn = 0;
  bool should_read = true;
  int prev_puyo_cnt = 0;
  while (fgets(buf, 1023, fp)) {
    if (buf[0] == '=') {
      if (match.eof_reason == Match::NONE)
        match.eof_reason = Match::END_MATCH;
      VLOG(2) << "New match: reason="
              << Match::kEofReasonStrs[match.eof_reason]
              << " num_decisions=" << match.decisions.size();
      should_read = true;
      turn = 0;
      prev_puyo_cnt = 0;
      matches->push_back(match);
      match = Match();
      continue;
    }
    if (!should_read) {
      continue;
    }
#ifdef OLD_PARSE_MOVIE
    if (strchr(buf, '5') && !(noeof_reason_mask & Match::OJAMA_PUYO_MASK))
#else
    if (strchr(buf, '1') && !(noeof_reason_mask & Match::OJAMA_PUYO_MASK))
#endif
    {
      should_read = false;
      match.eof_reason = Match::OJAMA_PUYO;
      continue;
    }
    if (turn >= num_turns) {
      should_read = false;
      match.eof_reason = Match::MANY_TURNS;
      continue;
    }

    turn++;
    CHECK_EQ(sscanf(buf, "%s%s%s", pbuf, nbuf, abuf), 3) << buf;

    LF problem;
    string next;
    LF answer;

    parseField(pbuf, &problem);
    parseNext(nbuf, &next);
    parseField(abuf, &answer);
    int chains, score, frames;
    answer.Simulate(&chains, &score, &frames);
    if (chains) {
      should_read = false;
      match.eof_reason = Match::VANISH_PUYO2;
      continue;
    }

    if ((problem.countPuyo() != prev_puyo_cnt ||
         answer.countPuyo() != prev_puyo_cnt + 2) &&
        !(noeof_reason_mask & Match::VANISH_PUYO_MASK)) {
#if 1
      LOG(INFO) << problem.countPuyo();
      LOG(INFO) << prev_puyo_cnt;
      LOG(INFO) << answer.countPuyo();
#endif
      should_read = false;
      match.eof_reason = Match::VANISH_PUYO;
      continue;
    }

    prev_puyo_cnt += 2;

    vector<LP> plans;
    problem.FindAvailablePlans(next, 1, &plans);
    Decision decision;
    for (size_t j = 0; j < plans.size(); j++) {
      if (answer.isEqual(plans[j].field)) {
        decision = plans[j].decision;
        break;
      }
    }
    if (!decision.IsValid()) {
      VLOG(1) << "inconsistent: " << problem.url() << ' ' << answer.url();
      should_read = false;
      match.eof_reason = Match::INCONSISTENT;
      continue;
    }

    match.seq += next[0] - 4 + 'A';
    match.seq += next[1] - 4 + 'A';
    match.seq += '-';
    match.decisions.push_back(decision);
  }
}
