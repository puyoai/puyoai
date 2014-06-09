#include "tsc.h"

#include <iostream>
#include <map>
#include <cmath>

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
#if defined(__i386__)
  asm volatile("rdtsc" : "=A" (tsc));
#elif defined(__x86_64__)
  unsigned int tscl, tsch;
  asm volatile("rdtsc":"=a"(tscl),"=d"(tsch));
  tsc = ((unsigned long long)tsch << 32)| tscl;
#else
# warning "Tsc::rdtsc() isn't implemented for thie architecture."
#endif
  return tsc;
}

map<string, vector<unsigned long long> > Tsc::data;
