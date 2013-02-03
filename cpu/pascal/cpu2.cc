#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <limits.h>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "ctrl.h"
#include "data.h"
#include "field.h"

using namespace std;

typedef pair<int, int> Point;
typedef set<Point> PointsToVanish;

Decision GetDecision(const Plan& plan) {
  const Plan* ret = &plan;
  while (ret->parent) {
    ret = ret->parent;
  }
  return ret->decision;
}

void CreateVanishPatternsInternal(int m[9][9], int depth,
                                  set<PointsToVanish>* ret) {
  if (depth == 3) {
    PointsToVanish tmp;
    for (int i = 1; i <= 7; i++) {
      for (int j = 1; j <= 7; j++) {
        if (m[i][j]) {
          tmp.insert(make_pair(i - 4, j - 4));
        }
      }
    }
    if (tmp.size() != 4) {
      exit(1);
    }
    ret->insert(tmp);
    return;
  }
  for (int i = 1; i <= 7; i++) {
    for (int j = 1; j <= 7; j++) {
      if (m[i][j]) {
        continue;
      }
      if (m[i-1][j] + m[i+1][j] + m[i][j-1] + m[i][j+1] == 0) {
        continue;
      }
      m[i][j] = 1;
      CreateVanishPatternsInternal(m, depth + 1, ret);
      m[i][j] = 0;
    }
  }
}

set<PointsToVanish> CreateVanishPatterns() {
  int m[9][9];
  memset(m, 0, sizeof(m));
  m[4][4] = 1;

  set<PointsToVanish> ret;
  CreateVanishPatternsInternal(m, 0, &ret);
  return ret;
}

map<PointsToVanish, map<PointsToVanish, int> >
GetVanishStartingPoints(const Field& field,
                        ofstream& ofs,
                        const set<PointsToVanish>& vanish_patterns) {
  map<PointsToVanish, map<PointsToVanish, int> > ret;
  for (int x = 1; x <= 6; x++) {
    for (int y = 1; y <= 12; y++) {
      char color = field.Get(x, y);
      if (color == EMPTY || color == OJAMA) {
        continue;
      }
      // We cannot vanish a puyo if it does not face an empty cell.
      if (field.Get(x-1, y) != EMPTY &&
          field.Get(x+1, y) != EMPTY &&
          field.Get(x, y+1) != EMPTY) {
        continue;
      }

      // GetHowToVanishPuyoAt(x, y);
      for (set<PointsToVanish>::const_iterator i =
               vanish_patterns.begin(); i != vanish_patterns.end(); i++) {
        // Match a given pattern with the field.
        bool applicable = true;
        PointsToVanish used_cells;
        PointsToVanish empty_cells;
        for (PointsToVanish::const_iterator j = i->begin();
             j != i->end(); j++) {
          int x1 = x + j->first;
          int y1 = y + j->second;
          if (!(1 <= x1 && x1 <= 6 && 1 <= y1 && y1 <= 13)) {
            applicable = false;
            break;
          }
          if (field.Get(x1, y1) == color) {
            used_cells.insert(make_pair(x1, y1));
          } else if (field.Get(x1, y1) == EMPTY) {
            empty_cells.insert(make_pair(x1, y1));
          } else {
            applicable = false;
            break;
          }
        }
        if (applicable) {
          // Simulate vanishment.
          Field f(field);
          for (PointsToVanish::iterator k = empty_cells.begin();
               k != empty_cells.end(); k++) {
            int x = k->first;
            int y = k->second;
            f.Set(x, y, color);
          }
          Field f2(f);
          int chains, score, frames;
          f.Simulate(&chains, &score, &frames);
          if (chains > 1) {
            if (chains > ret[used_cells][empty_cells]) {
              ret[used_cells][empty_cells] = chains;
            }
          }
        }
      }
    }
  }
  return ret;
}

double GetScore(const Field& f, ofstream& ofs) {
  // If there are too many colors in a small section, it's bad.
  double colors_score = 0;
  {
    float connection_scores[] = {0.0, 0.1, 0.3, 1.0, 0.0};

    for (int x = 0; x <= 6; x++) {
      for (int y = 0; y <= 13; y++) {
        int colors[8] = {0};
        for (int xx = 0; xx < 2; xx++) {
          for (int yy = 0; yy < 2; yy++) {
            colors[(int)f.Get(x + xx, y + yy)]++;
          }
        }
        for (int i = 4; i < 8; i++) {
          colors_score += connection_scores[colors[i]];
        }
      }
    }
  }

  // If 2 columns has too different height, it's bad.
  double height_score = 1.0;
  {
    double score_map[] = {10, 10, 9, 8, 6, 3, 2, 1.5, 1, 0.8, 0.7,
                          0.6, 0.5, 0.4, 0.3};
    int heights[7] = {0};
    for (int i = 1; i <= 6; i++) {
      for (int j = 1; j < 15; j++) {
        if (f.Get(i, j) == EMPTY) {
          heights[i] = j - 1;
          break;
        }
      }
    }
    for (int i = 1; i < 6; i++) {
      int diff = heights[i] - heights[i+1];
      if (diff < 0) diff = -diff;
      height_score *= score_map[diff];
    }

    double score[] = {1, 1, 1, 1, 0.9, 0.8, 0.8, 0.6, 0.5, 0.3, 0.1, 0.0, 0.0};
    for (int i = 1; i <= 6; i++) {
      height_score += score[heights[i]] * 5;
      if (i == 2 || i == 3 || i == 4) {
        if (f.Get(i, 9) != EMPTY) {
          height_score -= 10;
        }
      }
    }
  }

  // If the point is high, it's good.
  double rensa_score = 0.0;
  if (0) {
    Field f2(f);
    int chains;
    int score;
    int frames;
    f2.Simulate(&chains, &score, &frames);
    rensa_score = score / 400.0;
  }

  return (colors_score + 0.1) *
      (height_score + 0.1) *
      (rensa_score + 0.1);
}

// EMERGENCY MODE
Decision HandleEmergency(
    const Data& data, const vector<Plan>& plans, ofstream& ofs) {
  Decision my_decision = Decision();
  // [Score of minimum 3-rensa] - 1.
  int max_score = -1;

  for (unsigned int i = 0; i < plans.size(); i++) {
    const Plan& plan = plans[i];
    int score = plan.score;
    if (max_score < score) {
      max_score = score;
      my_decision = GetDecision(plan);
    }
  }
  return my_decision;
}

set<PointsToVanish> VANISH_PATTERNS;

void InitParameters() {
  VANISH_PATTERNS = CreateVanishPatterns();
}

Decision HandleNormalPlay(
    const Data& data, const vector<Plan>& plans, ofstream& ofs) {
  Decision my_decision = Decision();
  double max_score = -1;

  int best_plan_id = 0;
  for (unsigned int i = 0; i < plans.size(); i++) {
    const Plan& plan = plans[i];
    const Field& f = plan.field;

    // See how the field goes.
    map<PointsToVanish, map<PointsToVanish, int> > vanish_info =
        GetVanishStartingPoints(f, ofs, VANISH_PATTERNS);

    // chains, easiness.
    vector<double> possibility(20);
    for (map<PointsToVanish, map<PointsToVanish, int> >::iterator j =
             vanish_info.begin(); j != vanish_info.end(); j++) {
      double pattern_score = 1.0;

      double points_we_already_have[] = {0.0, 1.0, 4.0, 10.0};
      pattern_score *= points_we_already_have[j->first.size()];

      int patterns = j->second.size();
      pattern_score *= patterns;

      for (map<PointsToVanish, int>::iterator k = j->second.begin();
           k != j->second.end(); k++) {
        int chains = k->second;

        double easiness = 1.0;
        for (PointsToVanish::iterator l = k->first.begin();
             l != k->first.end(); l++) {
          // It takes some cost to fill an empty cell.
          easiness *= 0.1;
          int x = l->first;
          int y = l->second;
          while (f.Get(x, y) == EMPTY) {
            // It takes some extra cost to fill some other cells.
            easiness *= 0.7;
            y--;
          }
        }

        easiness *= pattern_score;
        if (possibility[chains] < easiness) {
          possibility[chains] += easiness;
        }
      }
    }

    const double multiplier[20] = {
      0.0, 0.0, 1.0, 4.0, 16.0, 64.0, 256.0, 1000.0,
      4000.0, 16000.0, 64000.0, 256000.0, 1000000.0,
      4000000.0, 16000000.0, 50.0, 100.0, 300, 1000, 10000
    };
    double score = 0.0001;
    for (int j = 0; j < 20; j++) {
      if (possibility[j] > 0.0) {
        ofs << j << "Rensa: " << possibility[j] << endl << flush;
      }
      score += possibility[j] * multiplier[j];
    }

    score *= GetScore(f, ofs);
    if (score > max_score) {
      max_score = score;
      my_decision = GetDecision(plan);
      best_plan_id = i;
    }
  }
  ofs << "[Result]" << endl << plans[best_plan_id].field.GetDebugOutput() << flush;

  return my_decision;
}

// argv[1] will have "Player1" for player 1, and "Player2" for player 2.
int main(int argc, char* argv[]) {
  // Logging.
  string name = string(argv[1]) + ".txt";
  ofstream ofs(name.c_str());

  ofs << "##### START #####" << endl << flush;
  // Make sure the CPU is connected to the duel server.
  // Echo back what we receive.
  // MAKE SURE to flush.
  Data data = Data::Get(ofs);
  cout << data.original << endl << flush;

  ofs << "##### SENT ECHO #####" << endl << flush;
  InitParameters();

  ofs << "##### INIT DONE #####" << endl << flush;

  // Main loop of the CPU.
  while (1) {
    data = Data::Get(ofs);
    ofs << "#### ID=" << data.id << " ####" << endl << flush;
    if (!data.users[0].HasControl() && data.nack.size() == 0) {
      ofs << "ID=" << data.id << endl << flush;
      cout << "ID=" << data.id << endl << flush;
      continue;
    }

    Decision my_decision = Decision();
    if (data.users[0].ojama >= 6 || data.users[0].field.Get(3, 9) != EMPTY) {
      // Think about the following 3 hands.
      vector<Plan> plans;
      data.users[0].field.FindAvailablePlans(2, data.users[0].yokoku.c_str(), &plans);
      my_decision = HandleEmergency(data, plans, ofs);
    } else {
      // Think about the following 1 hand.
      vector<Plan> plans;
      data.users[0].field.FindAvailablePlans(1, data.users[0].yokoku.c_str(), &plans);
      my_decision = HandleNormalPlay(data, plans, ofs);
    }

    ofs << "ID=" << data.id << " "
        << "X=" << my_decision.x << " "
        << "R=" << my_decision.r << " "
        << endl << flush;
    cout << "ID=" << data.id << " "
         << "X=" << my_decision.x << " "
         << "R=" << my_decision.r << " "
         << endl << flush;
  }

  return 0;
}
