#ifndef __CPU_HIROSHIMIZUNO_HIKUITOKORO_H__
#define __CPU_HIROSHIMIZUNO_HIKUITOKORO_H__

#include <sstream>
#include <string>

#include "../../core/decision.h"
#include "../../core/field_deprecated.h"

#define Field FieldDeprecated

using std::istringstream;
using std::ostringstream;
using std::pair;
using std::string;
using std::vector;

class Hikuitokoro {
 public:
  static int CalcFreedom(const Field& field) {
    int freedom = 0;
    int heights[8] = {0};
    for (int x = 1; x <= 6; ++x) {
      for (int y = 1; y <= 12; ++y) {
        const int puyo = field.Get(x, y);
        if (puyo & 4 == 0) { heights[x] = y - 1; break; }
        if (field.Get(x, y + 1) == puyo) { freedom += 1024; }
        if (field.Get(x + 1, y) == puyo) { freedom += 1024; }
      }
    }
    for (int x = 1; x < 6; ++x) {
      const int diff = heights[x] - heights[x + 1];
      freedom -= diff * diff;
    }
    return freedom;
  }
  string ProcessFrame(const string& line) {
    istringstream input(line);
    string id, field_string, nexts;
    for (string tmp; input >> tmp; ) {
      const string tag = tmp.substr(0, 3);
      if (tag == "ID=") { id = tmp.substr(3); }
      if (tag == "YF=") { field_string = tmp.substr(3); }
      if (tag == "YP=") { nexts = tmp.substr(3); }
    }
    const Field field(field_string);
    vector<pair<Decision, Field> > candidates;
    Field::GetPossibleFields(field, nexts[0], nexts[1], &candidates);
    int max_score = 699;
    int best_by_score = -1;
    int max_freedom = -987654321;
    int best_by_freedom = 0;
    for (int i = 0; i < candidates.size(); ++i) {
      const Field& f = candidates[i].second;
      Field f_simulate(f);
      int chain, score, time;
      f_simulate.Simulate(&chain, &score, &time);
      if (max_score < score) {
        max_score = score;
        best_by_score = i;
      } else if (score == 0) {
        const int freedom = CalcFreedom(f);
        if (max_freedom < freedom) {
          max_freedom = freedom;
          best_by_freedom = i;
        }
      }
    }
    const int best = best_by_score < 0 ? best_by_freedom : best_by_score;
    const Decision& decision = candidates[best].first;
    ostringstream output;
    output << "ID=" << id << " X=" << decision.x << " R=" << decision.r;
    return output.str();
  }
 private:
  vector<int> durations_;
};

#endif  // __CPU_HIROSHIMIZUNO_HIKUITOKORO_H__
