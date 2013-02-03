#include "main.h"

#include <iostream>
#include <fstream>
#include <limits.h>
#include <sstream>
#include <string>

#include "core/ctrl.h"
#include "core/field.h"
#include "core/plan.h"

using namespace std;

ofstream* of;

void FindAvailablePlansInternal(
    const Field& field, const Plan* parent, int depth, int max_depth,
    vector<Plan>* plans) {

  static const Decision decisions[] = {
    Decision(1, 2),
    Decision(2, 2),
    Decision(3, 2),
    Decision(4, 2),
    Decision(5, 2),
    Decision(6, 2),
    Decision(2, 3),
    Decision(3, 3),
    Decision(4, 3),
    Decision(5, 3),
    Decision(6, 3),

    Decision(1, 0),
    Decision(2, 0),
    Decision(3, 0),
    Decision(4, 0),
    Decision(5, 0),
    Decision(6, 0),
    Decision(1, 1),
    Decision(2, 1),
    Decision(3, 1),
    Decision(4, 1),
    Decision(5, 1),
  };

  char c1 = field.GetNextPuyo(depth * 2 + 0);
  char c2 = field.GetNextPuyo(depth * 2 + 1);
  int num_decisions = (c1 == c2) ? 11 : 22;

  // Cause of slowness?
  int heights[Field::MAP_WIDTH+1];
  for (int x = 1; x <= Field::WIDTH; x++) {
    heights[x] = 100;
    for (int y = 1; y <= Field::HEIGHT + 2; y++) {
      if (field.Get(x, y) == EMPTY) {
        heights[x] = y;
        break;
      }
    }
  }

  for (int i = 0; i < num_decisions; i++) {
    const Decision& decision = decisions[i];
    if (!Ctrl::isReachable(field, decision)) {
      continue;
    }

    Field next_field(field);

    int x1 = decision.x;
    int x2 = decision.x + (decision.r == 1) - (decision.r == 3);

    if (decision.r == 2) {
      next_field.Set(x2, heights[x2]++, c2);
      next_field.Set(x1, heights[x1]++, c1);
    } else {
      next_field.Set(x1, heights[x1]++, c1);
      next_field.Set(x2, heights[x2]++, c2);
    }
    heights[x1]--;
    heights[x2]--;
    int chains, score, frames;
    next_field.Simulate(&chains, &score, &frames);
    if (next_field.Get(3, 12) != EMPTY) {
      continue;
    }

    // Add a new plan.
    plans->push_back(Plan());
    Plan& plan = plans->at(plans->size() - 1);
    plan.field = next_field;
    plan.decision = decision;
    plan.parent = parent;
    if (parent) {
      plan.score = parent->score + score;
    } else {
      plan.score = score;
    }

    if (depth < max_depth - 1) {
      FindAvailablePlansInternal(next_field, &plan, depth + 1, max_depth, plans);
    }
  }
}

void FindAvailablePlans(const Field& field, int depth, vector<Plan>* plans) {
  plans->clear();
  plans->reserve(22 + 22*22 + 22*22*22);
  FindAvailablePlansInternal(field, NULL, 0, depth, plans);
}

void FindAvailablePlans(const Field& field, vector<Plan>* plans) {
  FindAvailablePlans(field, 3, plans);
}

double getScore(const Field& f, ofstream& ofs) {
  // If there are too many colors in a small section, it's bad.
  double colors_score = 0;
  if (0) {
    float connection_scores[] = {0.0, 0.0, 0.2, 0.4, 1.0, 1.2, 0.8, 0.4, 0.3, 0.3};

    for (int x = 1; x <= 4; x++) {
      for (int y = 1; y <= 11; y++) {
        int colors[8] = {0};
        for (int xx = 0; xx < 3; xx++) {
          for (int yy = 0; yy < 2; yy++) {
            colors[(int)f.Get(x + xx, y + yy)]++;
          }
        }
        for (int i = 4; i < 8; i++) {
          colors_score += connection_scores[colors[i]];
        }

        if (f.Get(x, y) == f.Get(x+2, y) &&
            f.Get(x+1, y) == EMPTY) {
          colors_score -= 4.0;
        }
      }
    }
  }

  {
    float connection_scores[] = {0.0, 0.0, 0.3, 0.8, 0.0};

    for (int x = 1; x <= 5; x++) {
      for (int y = 1; y <= 12; y++) {
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
  double height_score = 0.0;
  {
    double score_map[] = {1.5, 1.2, 0.8, 0.3, 0.1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
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
      height_score += score_map[diff];
    }

    double score[] = {1, 1, 1, 1, 0.9, 0.8, 0.8, 0.6, 0.5, 0.3, 0.1, 0.0, 0.0};
    for (int i = 1; i <= 6; i++) {
      height_score += score[heights[i]];
      if (i == 2 || i == 3 || i == 4) {
        if (f.Get(i, 9) != EMPTY) {
          //height_score -= 10;
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

  return colors_score + height_score + rensa_score;
}

void PascalCpu::GetDecision(
    const Data& data, Decision* decision, string* message) {
  // Logging.
  string name = "/tmp/test.txt";
  ofstream ofs(name.c_str());

  if (!data.HasControl(0)) {
    return;
  }

  ofs << "Received: " << data.id << endl << flush;
  ofs << data.player[0].field.GetDebugOutput() << endl << flush;

  vector<Plan> plans;
  FindAvailablePlans(data.player[0].field, 3, &plans);
  ofs << "Plans = " << plans.size() << endl << flush;

  // emergency.
  if (data.player[0].ojama > 6) {
    ofs << "EMERGENCY" << endl << flush;
    int min_score = INT_MAX;
    const Plan* my_plan = NULL;
    for (unsigned int i = 0; i < plans.size(); i++) {
      if (plans[i].parent && (plans[i].parent->parent == NULL)) {
        int score = plans[i].score;

        if (data.player[0].ojama * 70 < score &&
            score > min_score) {
          min_score = score;
          my_plan = &plans[i];
        }
      }
    }
    if (my_plan) {
      my_plan = my_plan->parent;
      *decision = my_plan->decision;
      stringstream buf;
      buf << min_score;
      *message = buf.str();
      ofs << "ID=" << data.id << " X=" << decision->x << " R=" << decision->r
          << " SCORE=" << min_score << endl << flush;
      return;
    }
  }

  double max_score = -1;
  const Plan* my_plan = NULL;
  for (unsigned int i = 0; i < plans.size(); i++) {
    double score = plans[i].score / 10.0 + getScore(plans[i].field, ofs);
    if (score > max_score) {
      max_score = score;
      my_plan = &plans[i];
      ofs << "score: " << score << " / " << "planId = " << i << " / ["
          << plans[i].decision.x << ", " << plans[i].decision.r << "]"
          << endl;
    }
  }
  ofs << "Max score = " << max_score << endl;
  if (my_plan) {
    ofs << my_plan->field.GetDebugOutput() << endl << flush;
    while (my_plan->parent != NULL) {
      my_plan = my_plan->parent;
    }
    *decision = my_plan->decision;
  }
  ofs << "decision = [" << decision->x << ", " << decision->r << "]" << endl;

  stringstream buf;
  buf << max_score;
  *message = buf.str();
  ofs << "ID=" << data.id << " X=" << decision->x << " R=" << decision->r
      << " SCORE=" << max_score << endl << flush;
}

// argv[1] will have "Player1" for player 1, and "Player2" for player 2.
int main(int argc, char* argv[]) {
  // Logging.
  string name = "/tmp/" + string(argv[1]) + ".txt";
  ofstream ofs(name.c_str());

  PascalCpu cpu;
  cpu.Run();
}
