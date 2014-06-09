#include "ratingstats.h"

#include <math.h>
#include <stdio.h>

#include <algorithm>
#include <functional>

#include <gflags/gflags.h>
#include <glog/logging.h>

void Stat::merge(const Stat& from) {
  for (map<int, int>::const_iterator iter = from.turns_.begin();
       iter != from.turns_.end(); ++iter) {
    CHECK(turns_.find(iter->first) == turns_.end());
    turns_[iter->first] = iter->second;
  }
}

const map<int, int>& Stat::turns() const{
  return turns_;
}

void Stat::add(int i, int t) {
  if (turns_.find(i) == turns_.end()) {
    turns_[i] = t;
  }
}

int Stat::num() const {
  return turns_.size();
}

int Stat::get(int i) const {
  map<int, int>::const_iterator iter = turns_.find(i);
  if (iter != turns_.end()) {
    return iter->second;
  }
  return -1;
}

void Stat::show(int total_count) const {
  vector<float> a;
  for (map<int, int>::const_iterator iter = turns_.begin();
       iter != turns_.end(); ++iter) {
    a.push_back(iter->second);
  }
  printf("turn: ");
  showStat(a);
  if (total_count > 25) {
    double p = (float)a.size() / total_count;
    double interval = 2.0*sqrt(p*(1.0-p)/total_count);
    printf(" prob: %.2f%% (%.2f%%,%.2f%%)\n",
           p*100.0,
           (p + interval)*100.0,
           (p - interval)*100.0);
  } else {
    printf(" prob: %.2f%%\n", (float)a.size() * 100 / total_count);
  }
}

void RatingStats::add_invalid_stat(int i, int t) {
  invalid_stat.add(i, t);
}

void RatingStats::add_dead_stat(int i, int t) {
  dead_stat.add(i, t);
}

void RatingStats::add_elapsed_msec(float ms) {
  cpuMsecs.push_back(ms);
}

void RatingStats::add_chain_stats(int c, int i, int t) {
  chain_stats[c].add(i, t);
}

void RatingStats::add_max_score(double maxScore, const string& url) {
  if (maxScores.empty() || max_score < maxScore) {
    max_score = maxScore;
    max_url = url;
  }
  maxScores.push_back(maxScore);
}

void RatingStats::add_chigiri_frames(int f) {
  chigiri_frames.push_back(f);
}

void RatingStats::Print() {
  if (invalid_stat.num()) {
    printf("INVALID: ");
    invalid_stat.show(total_count);
  }
  if (dead_stat.num()) {
    printf("DEAD: ");
    dead_stat.show(total_count);
  }
  for (int c = 2; c < 20; c++) {
    if (!chain_stats[c].num())
      break;
    printf("%d RENSA: ", c);
    chain_stats[c].show(total_count);
  }

  printf("%d\n", (int)chigiri_frames.size());
  printf("%f\n", chigiri_frames[0]);
  printf("CHIGIRI: ");
  Stat::showStat(chigiri_frames);
  printf("\nSCORE: ");
  Stat::showStat(maxScores);
  printf("\nMaxScore: %.2f\n", max_score);
  printf("MaxScoreURL: %s\n", max_url.c_str());

  printf("Time(ms): ");
  Stat::showStat(cpuMsecs);
  puts("");
}

void Stat::showStat(const vector<float>& a) {
  float tot = 0;
  float mx = 0;
  float mn = 1e99;
  for (size_t i = 0; i < a.size(); i++) {
    float v = a[i];
    tot += v;
    mx = max(mx, v);
    mn = min(mn, v);
  }

  float avg = tot / a.size();
  float diff_sq_sum = 0;
  for (size_t i = 0; i < a.size(); i++) {
    float v = a[i] - avg;
    diff_sq_sum += v * v;
  }

  float std_dev = 0;
  if (a.size() > 1) {
    std_dev = sqrt(diff_sq_sum / (a.size() - 1));
  }

  printf("avg=%.2f dev=%.2f max=%.2f min=%.2f",
         avg, std_dev, mx, mn);
}

RatingStats::RatingStats() : max_score(0) {
  total_count = 0;
}

void RatingStats::merge(const RatingStats& from) {
  for (size_t i = 0; i < from.maxScores.size(); ++i) {
    maxScores.push_back(from.maxScores[i]);
  }
  for (size_t i = 0; i < from.cpuMsecs.size(); ++i) {
    cpuMsecs.push_back(from.cpuMsecs[i]);
  }
  copy(from.chigiri_frames.begin(), from.chigiri_frames.end(),
       back_inserter(chigiri_frames));

  invalid_stat.merge(from.invalid_stat);
  dead_stat.merge(from.dead_stat);
  for (int i = 0; i < 20; ++i) {
    chain_stats[i].merge(from.chain_stats[i]);
  }

  if (from.max_score > max_score) {
    max_score = from.max_score;
    max_url = from.max_url;
  }
  total_count += from.total_count;
}
