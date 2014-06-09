#ifndef HAMAJI_RATINGSTATS_H_
#define HAMAJI_RATINGSTATS_H_

#include <map>
#include <string>
#include <vector>

#include "base.h"

class Stat {
 public:
  void merge(const Stat& from);
  void add(int i, int t);
  int num() const;
  void show(int total_count) const;
  static void showStat(const vector<float>& a);
  int get(int i) const;
  const map<int, int>& turns() const;

 private:
  map<int, int> turns_;
};

class RatingStats {
 public:
  RatingStats();
  void add_invalid_stat(int i, int t);
  void add_dead_stat(int i, int t);
  void add_elapsed_msec(float ms);
  void add_chain_stats(int c, int i, int t);
  void add_max_score(double maxScore, const string& url);
  void add_chigiri_frames(int f);
  void Print();
  void merge(const RatingStats& from);

  Stat invalid_stat, dead_stat;
  Stat chain_stats[20];
  vector<float> maxScores;
  vector<float> cpuMsecs;
  vector<float> chigiri_frames;
  int total_count;
  string max_url;
  double max_score;
};

#endif  // HAMAJI_RATINGSTATS_H_
