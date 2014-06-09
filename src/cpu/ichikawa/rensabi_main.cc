// GTR example: http://bit.ly/JULjtw

#include <iostream>
#include <string>
#include <map>
#include <algorithm>
#include <iterator>
#include <set>
#include <cassert>
#include <bitset>
#include <sstream>
#include <fstream>
#include <climits>
#include <gflags/gflags.h>

#include "core/decision.h"
#include "deprecated_field.h"

using namespace std;
typedef DeprecatedField Field;

// TODO Extract common logic with gtr_main.cc to another file.

const char NORMAL_COLOR_BEGIN = 4;
const char NORMAL_COLOR_END = 8;
const char IMPOSSIBLE = NORMAL_COLOR_END;

DEFINE_int32(goal_chain_len, 9, "");
DEFINE_bool(hate_small_chain, false, "");
DEFINE_bool(look_at_next_next, true, "");

struct Point {
  int x;
  int y;
  explicit Point(int ax = 0, int ay = 0) : x(ax), y(ay) { }
};

vector<Point> Points(const Point& p1, const Point& p2) {
  vector<Point> points;
  points.push_back(p1);
  points.push_back(p2);
  return points;
}

struct State {
  unsigned int i_activated : 1;
  unsigned int other_activated : 1;
  unsigned int i_next_next : 1;
  unsigned int other_next_next : 1;
  unsigned int i_fixed : 1;
  unsigned int other_fixed : 1;
  unsigned int i_won : 1;
  unsigned int other_won : 1;
  unsigned int i_chain_finished : 1;
  unsigned int other_chain_finished : 1;
};

struct Input {
  int id;
  FieldWithColorSequence my_field;
  Point my_current_point;
  int num_my_ojamas;
  State state;
  string original;
  Input() : id(0), num_my_ojamas(0) {}
  Input(const Input& other)
      : id(other.id),
        my_field(other.my_field),
        num_my_ojamas(other.num_my_ojamas),
        state(other.state),
        original(other.original) {
    my_field.SetColorSequence(other.my_field.GetColorSequence());
  }
};

class Chunk {
 public:
  explicit Chunk(FieldWithColorSequence* f) : field_(f), fill_color_(EMPTY) { }
  Chunk& Add(const Point& point);
  Chunk& Add(int x, int y);
  void Clear();
  void Dream(char* next_color);
  char fill_color() const { return fill_color_; }
  const vector<Point>& points() const { return points_; }

 private:
  FieldWithColorSequence* field_;
  vector<Point> points_;
  char fill_color_;
};

void GetInput(Input* input, ofstream& log) {
  string str;
  getline(cin, str, '\n');

  istringstream iss(str);
  string tmp;
  input->original = str;
  //log << "[" << input->original << "]" << endl << flush;
  string my_field_str, my_puyos_str;
  while(getline(iss, tmp, ' ')) {
    if (tmp.substr(0, 3) == "ID=") {
      istringstream istr(tmp.c_str() + 3);
      istr >> input->id;
    } else if (tmp.substr(0, 3) == "YF=") {
      istringstream istr(tmp.c_str() + 3);
      istr >> my_field_str;
    } else if (tmp.substr(0, 3) == "YP=") {
      istringstream istr(tmp.c_str() + 3);
      istr >> my_puyos_str;
    } else if (tmp.substr(0, 3) == "YX=") {
      istringstream istr(tmp.c_str() + 3);
      istr >> input->my_current_point.x;
    } else if (tmp.substr(0, 3) == "YY=") {
      istringstream istr(tmp.c_str() + 3);
      istr >> input->my_current_point.y;
    } else if (tmp.substr(0, 3) == "YO=") {
      istringstream istr(tmp.c_str() + 3);
      istr >> input->num_my_ojamas;
    } else if (tmp.substr(0, 6) == "STATE=") {
      istringstream istr(tmp.c_str() + 6);
      unsigned int val;
      istr >> val;
      input->state = *reinterpret_cast<State*>(&val);
    }
  }
  input->my_field = FieldWithColorSequence(my_field_str);
  input->my_field.SetColorSequence(my_puyos_str);
}

bool CanPut(const FieldWithColorSequence& field, int x) {
  int path_left = 0;
  int path_right = 0;
  if (x >= 3) {
    path_left = 3;
    path_right = x - 1;
  } else {
    path_left = x + 1;
    path_right = 3;
  }
  for (int px = path_left; px <= path_right; ++px) {
    if (field.Get(px, FieldWithColorSequence::HEIGHT) != EMPTY) {
      // Cannot reach the column.
      return false;
    }
  }
  return true;
}

bool CanPut(const FieldWithColorSequence& field, const Point& current_point, const Decision& decision) {
  // This should be current_point.y when this is the first action for the puyo.
  // Not sure this is OK for the puyo already falling.
  int active_y = current_point.y;
  if (active_y < 1) return false;
  int path_left = min(current_point.x, decision.x);
  int path_right = max(current_point.x, decision.x);
  for (int x = path_left; x <= path_right; ++x) {
    if (field.Get(x, active_y) != EMPTY) return false;
  }
  return true;
}

bool Put(FieldWithColorSequence* field, int x, char color, int* result_x = NULL, int* result_y = NULL) {
  if (x < 1 || x > FieldWithColorSequence::WIDTH) return false;
  for (int y = 1; y <= FieldWithColorSequence::HEIGHT; ++y) {
    if (field->Get(x, y) == EMPTY) {
      field->Set(x, y, color);
      if (result_x) *result_x = x;
      if (result_y) *result_y = y;
      return true;
    }
  }
  return false;
}

bool Put(
    FieldWithColorSequence* field, const Decision& decision, const pair<char, char>& puyos,
    int* first_x = NULL, int* first_y = NULL, int* second_x = NULL, int* second_y = NULL) {
  if (!CanPut(*field, decision.x)) return false;
  switch (decision.r) {
    case 0:
      if (!Put(field, decision.x, puyos.first, first_x, first_y)) return false;
      if (!Put(field, decision.x, puyos.second, second_x, second_y)) return false;
      break;
    case 1:
      if (!Put(field, decision.x, puyos.first, first_x, first_y)) return false;
      if (!Put(field, decision.x + 1, puyos.second, second_x, second_y)) return false;
      break;
    case 2:
      if (!Put(field, decision.x, puyos.second, second_x, second_y)) return false;
      if (!Put(field, decision.x, puyos.first, first_x, first_y)) return false;
      break;
    case 3:
      if (!Put(field, decision.x, puyos.first, first_x, first_y)) return false;
      if (!Put(field, decision.x - 1, puyos.second, second_x, second_y)) return false;
      break;
    default:
      assert(false);
      break;
  }
  return true;
}

bool Dream(FieldWithColorSequence* field, int x, int y, char color) {
  if (field->Get(x, y) == EMPTY && CanPut(*field, x)) {
    field->Set(x, y, color);
    return true;
  } else {
    return false;
  }
}

bool Dream(FieldWithColorSequence* field, const Point& point, char color) {
  return Dream(field, point.x, point.y, color);
}

Chunk& Chunk::Add(const Point& point) {
  points_.push_back(point);
  return *this;
}

Chunk& Chunk::Add(int x, int y) {
  points_.push_back(Point(x, y));
  return *this;
}

void Chunk::Clear() {
  points_.clear();
}

void Chunk::Dream(char* next_color) {
  bool possible = true;
  for (int i = 0; i < int(points_.size()); ++i) {
    char color = field_->Get(points_[i].x, points_[i].y);
    if (color >= NORMAL_COLOR_BEGIN && color < NORMAL_COLOR_END) {
      if (fill_color_ == EMPTY) {
        fill_color_ = color;
      } else if (color != fill_color_) {
        possible = false;
        break;
      }
    } else if (color != EMPTY) {
      possible = false;
      break;
    }
  }
  //cout << "fc: " << int(fill_color_) << endl;
  //cout << "possible: " << possible << endl;
  if (fill_color_ == EMPTY || !possible) {
    fill_color_ = (*next_color)++;
  }
  for (int i = 0; i < int(points_.size()); ++i) {
    ::Dream(field_, points_[i], fill_color_);
  }
}

string FieldToStr(const FieldWithColorSequence& field) {
  string str;
  for (int y = FieldWithColorSequence::HEIGHT; y >= 1; --y) {
    for (int x = 1; x <= FieldWithColorSequence::WIDTH; ++x) {
      char color = field.Get(x, y);
      if (color == EMPTY) {
        str += ' ';
      } else if (color == OJAMA) {
        str += '@';
      } else {
        str += static_cast<char>('A' + color - NORMAL_COLOR_BEGIN);
      }
    }
    str += '\n';
  }
  return str;
}

bool IsNormalColor(char color) {
  return color >= NORMAL_COLOR_BEGIN && color < NORMAL_COLOR_END;
}

bool CreateKagiChunk(
    const FieldWithColorSequence& field,
    const Point& bottom,
    const Point& prev_remain,
    bool force_second_remain,
    Chunk* chunk,
    Point* remain) {
  char color = EMPTY;
  if (field.Get(bottom.x, bottom.y) != EMPTY) {
    color = field.Get(bottom.x, bottom.y);
  }
  if (color == EMPTY && field.Get(prev_remain.x, prev_remain.y) != EMPTY) {
    color = field.Get(prev_remain.x, prev_remain.y);
  }
  char second_color = field.Get(bottom.x, bottom.y + 1);
  char third_color = field.Get(bottom.x, bottom.y + 2);
  remain->x = bottom.x;
  if (!force_second_remain &&
      IsNormalColor(color) &&
      ((third_color != EMPTY && third_color != color) ||
       second_color == color)) {
    remain->y = bottom.y + 2;
  } else {
    remain->y = bottom.y + 1;
  }
  for (int y = bottom.y; y < bottom.y + 4; ++y) {
    if (y != remain->y) {
      chunk->Add(bottom.x, y);
    }
  }
  chunk->Add(prev_remain.x, prev_remain.y);

  if (color != EMPTY && !IsNormalColor(color)) return false;
  for (int i = 0; i < int(chunk->points().size()); ++i) {
    char pc = field.Get(chunk->points()[i].x, chunk->points()[i].y);
    if (color == EMPTY) {
      color = pc;
    } else if (pc != EMPTY && pc != color) {
      return false;
    }
  }
  return true;
}

void Dream(
    FieldWithColorSequence* field, Point* fire_point, char* fire_color, vector<Chunk>* chunks, bool debug = false) {
  chunks->clear();

  Chunk chunk(field);
  chunk.Add(2, 1).Add(2, 2).Add(3, 1);
  chunks->push_back(chunk);

  char next_color = NORMAL_COLOR_END;
  for (int i = 0; i < int(chunks->size()); ++i) {
    (*chunks)[i].Dream(&next_color);
  }

  *fire_point = Point(1, 1);
  *fire_color = chunks->back().fill_color();
}

int GetMaxChainLen(const FieldWithColorSequence& work_field, int* num_valuable_puyos, bool debug = false) {
  FieldWithColorSequence dream_field = work_field;
  Point fire_point;
  char fire_color = 0;
  vector<Chunk> chunks;
  Dream(&dream_field, &fire_point, &fire_color, &chunks, debug);
  if (debug) {
    cout << "work:" << endl << FieldToStr(work_field) << endl;
    cout << "dream:" << endl << FieldToStr(dream_field) << endl;
  }
  int chains, score, frames;
  dream_field.Simulate(&chains, &score, &frames);
  Dream(&dream_field, fire_point, fire_color);
  dream_field.Simulate(&chains, &score, &frames);
  if (debug) cout << "chains: " << chains << endl;

  *num_valuable_puyos = 0;
  for (int i = 0; i < int(chunks.size()); ++i) {
    for (int j = 0; j < int(chunks[i].points().size()); ++j) {
      const Point& point = chunks[i].points()[j];
      char color = work_field.Get(point.x, point.y);
      if (IsNormalColor(color)) {
        ++*num_valuable_puyos;
      }
    }
  }

  return chains;
}

int GarbagePenalty(const FieldWithColorSequence& field) {
  int penalty = 0;
  for (int x = 1; x <= 6; ++x) {
    for (int y = 5; y <= FieldWithColorSequence::HEIGHT; ++y) {
      if (field.Get(x, y) != EMPTY) {
        penalty += y;
      }
    }
  }
  return penalty;
}

void DropOjamas(FieldWithColorSequence* field, int num_ojamas) {
  int lines = (num_ojamas + 3) / 6;
  for (int x = 1; x <= FieldWithColorSequence::WIDTH; ++x) {
    int bottom = 1;
    for (; bottom <= FieldWithColorSequence::HEIGHT + 2; ++bottom) {
      if (field->Get(x, bottom) == EMPTY) break;
    }
    int top = min(bottom + lines, FieldWithColorSequence::HEIGHT + 3);
    for (int y = bottom; y < top; ++y) {
      field->Set(x, y, OJAMA);
    }
  }
}

int FirstColumnBonus(const FieldWithColorSequence& field) {
  return 0;
  /*
  for (int y = 1; y <= FieldWithColorSequence::HEIGHT; ++y) {
    if (field.Get(1, y) == EMPTY) {
      return FieldWithColorSequence::HEIGHT - y;
    }
  }
  return 0;
  */
}

int GetDreamHappiness(const FieldWithColorSequence& field, bool debug = false) {
  int num_valuable_puyos = 0;
  int max_chain_len = GetMaxChainLen(field, &num_valuable_puyos, debug);
  return FirstColumnBonus(field) * 100000000 +
      max_chain_len * 1000000 +
      num_valuable_puyos * 1000 +
      (1000 - GarbagePenalty(field));
}

int GetHappiness(
    const FieldWithColorSequence& field,
    const vector<pair<char, char> >& puyos_list,
    const vector<Decision>& decisions,
    const vector<int>& num_ojamas_list,
    bool debug = false) {
  assert(decisions.size() >= 2);
  assert(puyos_list.size() >= decisions.size());
  assert(num_ojamas_list.size() >= decisions.size());

  int max_chains = 0;
  FieldWithColorSequence work_field = field;
  int step = 0;
  for (; step < int(decisions.size()); ++step) {
    if (!Put(&work_field, decisions[step], puyos_list[step])) break;
    int chains = 0;
    int score = 0;
    int frames = 0;
    work_field.Simulate(&chains, &score, &frames);
    // This check is before DropOjamas() because current DropOjamas() doesn't consider 相殺.
    if (work_field.Get(3, FieldWithColorSequence::HEIGHT) != EMPTY) break;
    max_chains = max(max_chains, chains);
    DropOjamas(&work_field, num_ojamas_list[step]);
  }

  int immediate_happiness =
      FirstColumnBonus(field) * 100000000 +
      max_chains * 1000000 +
      (1000 - GarbagePenalty(work_field));
  if (step == 0) {
    return 0;
  } else if (step < int(decisions.size())) {
    return immediate_happiness;
  } else {
    int dream_happiness = GetDreamHappiness(work_field, debug);
    if (FLAGS_hate_small_chain) {
      return max_chains > 0 ? immediate_happiness : dream_happiness;
    } else {
      return max(immediate_happiness, dream_happiness);
    }
  }
}

void EvaluateDecisions(
    const Input& input,
    bool ojama_comming,
    const vector<pair<char, char> >& puyos_list,
    int cx, int cr, int nx, int nr, int nnx, int nnr,
    int* max_happiness,
    vector<Decision>* max_decisions) {
  vector<Decision> decisions;
  decisions.push_back(Decision(cx, cr));
  decisions.push_back(Decision(nx, nr));
  if (nnx >= 0 && nnr >= 0) {
    decisions.push_back(Decision(nnx, nnr));
  }
  vector<int> num_ojamas_list(3);
  if (ojama_comming) {
    num_ojamas_list[0] = input.num_my_ojamas;
  } else {
    num_ojamas_list[1] = input.num_my_ojamas;
  }
  if (!CanPut(input.my_field, input.my_current_point, decisions[0])) return;
  int happiness = GetHappiness(
      input.my_field, puyos_list, decisions, num_ojamas_list);
  //log << "happiness: " << happiness << endl;
  //log << new_field.GetDebugOutput() << endl;
  if (happiness > *max_happiness) {
    *max_decisions = decisions;
    *max_happiness = happiness;
  }
}

void Act(const Input& input, bool ojama_comming, Decision* decision, ostream& log) {
  const FieldWithColorSequence& field = input.my_field;
  vector<pair<char, char> > puyos_list;
  for (int i = 0; i < 3; ++i) {
    puyos_list.push_back(make_pair(field.GetNextPuyo(2 * i), field.GetNextPuyo(2 * i + 1)));
  }
  int max_happiness = 0;
  vector<Decision> max_decisions;
  for (int cx = 1; cx <= FieldWithColorSequence::WIDTH; ++cx) {
    for (int cr = 0; cr < 4; ++cr) {
      for (int nx = 1; nx <= FieldWithColorSequence::WIDTH; ++nx) {
        for (int nr = 0; nr < 4; ++nr) {
          if (FLAGS_look_at_next_next) {
            for (int nnx = 1; nnx <= FieldWithColorSequence::WIDTH; ++nnx) {
              for (int nnr = 0; nnr < 4; ++nnr) {
                EvaluateDecisions(
                    input, ojama_comming, puyos_list,
                    cx, cr, nx, nr, nnx, nnr,
                    &max_happiness, &max_decisions);
              }
            }
          } else {
            EvaluateDecisions(
                input, ojama_comming, puyos_list,
                cx, cr, nx, nr, -1, -1,
                &max_happiness, &max_decisions);
          }
        }
      }
    }
  }

  if (!max_decisions.empty()) {
    assert(max_decisions.size() >= 1);
    assert(puyos_list.size() >= max_decisions.size());
    FieldWithColorSequence new_field = field;
    for (int i = 0; i < int(max_decisions.size()); ++i) {
      Put(&new_field, max_decisions[i], puyos_list[i]);
    }
    //log << "current:" << endl;
    //log << input.my_field.GetDebugOutput() << endl;
    log << "max_happiness: " << max_happiness << endl;
    log << new_field.GetDebugOutput() << endl;

    int chains, score, frames;
    new_field.Simulate(&chains, &score, &frames);
    Point fire_point;
    char fire_color;
    vector<Chunk> chunks;
    Dream(&new_field, &fire_point, &fire_color, &chunks);
    log << "dream:" << endl;
    log << FieldToStr(new_field) << endl;
    *decision = max_decisions[0];

  } else {
    decision->x = -1;
    decision->r = -1;
  }
}

bool chain_finished = true;
bool ojama_comming = false;

void GetDecision(const Input& input, Decision* decision, ostream& log) {
  /*
  log
    << input.state.i_activated
    << input.state.other_activated
    << input.state.i_next_next
    << input.state.other_next_next
    << input.state.i_fixed
    << input.state.other_fixed
    << input.state.i_won
    << input.state.other_won
    << input.state.i_chain_finished
    << input.state.other_chain_finished << endl;
  log << input.my_field.GetDebugOutput() << endl;
  */
  if (input.id == 1 || input.state.i_chain_finished) {
    chain_finished = true;
  }
  if (input.state.other_chain_finished && input.num_my_ojamas > 0) {
    ojama_comming = true;
  }

  if (input.state.i_activated && (chain_finished || ojama_comming)) {
    if (ojama_comming) log << "OJAMA!" << endl;
    Act(input, ojama_comming, decision, log);
    chain_finished = false;
    ojama_comming = false;
  } else {
    decision->x = -1;
    decision->r = -1;
  }
}

void TestHappiness(const string& field_str) {
  FieldWithColorSequence field(field_str);
  GetDreamHappiness(field, true);
  cout << "------------------" << endl;
}

// argv[1] will have "Player1" for player 1, and "Player2" for player 2.
int main(int argc, char* argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  if (argc < 2) {
    cerr << "Usage: gtr_main Player1" << endl;
    return 1;
  }
  string arg = argv[1];
  assert(FLAGS_goal_chain_len >= 8 && FLAGS_goal_chain_len <= 12);

  if (arg == "test") {
    TestHappiness("");

  } else if (arg == "exp") {

  } else {
    // Logging.
    string name = arg + ".txt";
    ofstream log(name.c_str(), ios_base::out | ios_base::app);
    log << "------------------" << endl;

    // Make sure the CPU is connected to the duel server.
    // Echo back what we receive.
    // MAKE SURE to flush.
    Input input;
    GetInput(&input, log);
    cout << input.original << endl << flush;

    while (1) {
      GetInput(&input, log);
      //log << "Received: " << input.original << endl << flush;
      Decision decision;
      GetDecision(input, &decision, log);
      if (decision.x >= 0) {
        cout << "ID=" << input.id
            << " X=" << decision.x << " R=" << decision.r << endl << flush;
        log << "Output: ID=" << input.id << " X=" << decision.x << " R=" << decision.r << endl << flush;
      }
    }

  }
  return 0;
}
