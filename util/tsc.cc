#include "tsc.h"

#include <iostream>
#include <map>
#include <cmath>

#include "util/Config.h"

using namespace std;

Tsc::Tsc(const string& key) {
  key_ = key;
  data.insert(make_pair(key, vector<unsigned long long>()));
  st_ = rdtsc();
}

Tsc::~Tsc() {
  unsigned long long ed = rdtsc();
  if (ed > st_) {
    Tsc::record(key_, ed - st_);
  }
}

void Tsc::record(const string& key, unsigned long long diff) {
  data[key].push_back(diff);
}

void Tsc::GetStatistics(const string& key, double* average, double* rmsd) {
  vector<unsigned long long>& list = data[key];
  int n = list.size();
  double sum = 0.0;
  int ignored = 0;
  for (int i = 0; i < n; i++) {
    if (list[i] < 0) {
      ignored++;
      continue;
    }
    sum += list[i];
  }
  *average = sum / (n - ignored);

  double diff_square_sum = 0.0;
  for (int i = 0; i < n; i++) {
    if (list[i] < 0) {
      continue;
    }
    diff_square_sum += (list[i] - *average) * (list[i] - *average);
  }
  *rmsd = pow(diff_square_sum / (n - ignored), 0.5);
}

unsigned long long Tsc::rdtsc() {
  unsigned long long tsc = 0;
#ifdef TSC_ALT_IMPL
  asm volatile("rdtsc" : "=A" (tsc));
#else
  // On 64bit built program and other environment where you see an error at
  // this line, run:
  //   $ CXXFLAGS='-DTSC_ALT_IMPL' make
  // to use the alternative implementation above.
  asm volatile("rdtsc; shlq $32,%%rdx; orq %%rdx,%%rax" : "=A" (tsc) :: "%rdx");
#endif
  return tsc;
}

map<string, vector<unsigned long long> > Tsc::data;
