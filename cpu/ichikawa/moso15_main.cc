#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <map>
#include <algorithm>
#include <iterator>
#include <set>
#include <cassert>
#include <bitset>

#include "queue.h"
#include "../../core/field.h"
#include "../../core/decision.h"

using namespace std;

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
  Field my_field;
  State state;
  string original;
  Input() : id(0) {}
  Input(const Input& other)
      : id(other.id), my_field(other.my_field), state(other.state), original(other.original) {
    my_field.SetColorSequence(other.my_field.GetColorSequence());
  }
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
    } else if (tmp.substr(0, 6) == "STATE=") {
      istringstream istr(tmp.c_str() + 6);
      unsigned int val;
      istr >> val;
      input->state = *reinterpret_cast<State*>(&val);
    }
  }
  input->my_field = Field(my_field_str);
  input->my_field.SetColorSequence(my_puyos_str);
}

struct ChainKey {
  int x;
  int y;
  char color;
  explicit ChainKey(int a_x = 0, int a_y = 0, char a_color = 0) : x(a_x), y(a_y), color(a_color) {}
  friend bool operator<(const ChainKey& lhs, const ChainKey& rhs) {
    if (lhs.x < rhs.x) return true;
    if (lhs.x > rhs.x) return false;
    if (lhs.y < rhs.y) return true;
    if (lhs.y > rhs.y) return false;
    return lhs.color < rhs.color;
  }
};

const char NORMAL_COLOR_BEGIN = 4;
const char NORMAL_COLOR_END = 8;
//const int NUM_CHAIN_DATA = 10000;
//const char* CHAIN_DATA_PATH = "cpu/ichikawa/data/15chain.1kx10.txt";
const int NUM_CHAIN_DATA = 10000;
const char* CHAIN_DATA_PATH = "cpu/ichikawa/data/5chain.1kx10.txt";
//const int NUM_CHAIN_DATA = 100000;
//const char* CHAIN_DATA_PATH = "cpu/ichikawa/data/5chain.1kx100.txt";


bool Put(Field* field, int x, char color, int* result_x = NULL, int* result_y = NULL) {
  if (x < 1 || x > Field::WIDTH) return false;
  for (int y = 1; y <= Field::HEIGHT; ++y) {
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
    Field* field, const Decision& decision, const pair<char, char>& puyos,
    int* first_x = NULL, int* first_y = NULL, int* second_x = NULL, int* second_y = NULL) {
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

bool Put(
    Field* field,
    const vector<Decision>& decisions,
    const vector<pair<char, char> > & puyos_list) {
  assert(decisions.size() == puyos_list.size());
  for (int i = 0; i < int(decisions.size()); ++i) {
    if (!Put(field, decisions[i], puyos_list[i])) {
      return false;
    }
  }
  return true;
}

struct FieldLess {
  bool operator()(const Field& lhs, const Field& rhs) {
    for (int y = 1; y <= Field::HEIGHT; ++y) {
      for (int x = 1; x <= Field::WIDTH; ++x) {
        if (lhs.Get(x, y) < rhs.Get(x, y)) return true;
        if (lhs.Get(x, y) > rhs.Get(x, y)) return false;
      }
    }
    return false;
  }
};

class ChainDatabase {
 public:
  typedef bitset<NUM_CHAIN_DATA> ChainSet;
  
  struct CacheValue {
    vector<ChainSet> perm_to_matches;
  };
  
  typedef map<Field, CacheValue, FieldLess> Cache;
  
  explicit ChainDatabase(ostream& log);
  void StartDecisionThread();
  static void* DecisionThread(void* arg);
  void Act(const Input& input);
  void Decide(
      const Field& field,
      vector<Decision>* decisions) const;
  int CountMatches(
      const Field& prev_field,
      const pair<char, char>& puyos,
      const Decision& decision) const;
  int CountMatches(const Field& target) const;
  void GetMatches(
      const Field& target, const vector<char>& color_map, ChainSet* matches) const;
  const Decision& current_decision() const { return current_decision_; }
  
 private:
  // Used in main thread
  Decision current_decision_;
  pthread_t decision_thread_;
  
  // Used in decision thread
  map<ChainKey, ChainSet> index_;
  mutable Cache cache_;
  
  // Used in both threads
  Queue<Input> pending_inputs_;
  Queue<Decision> pending_decisions_;
  ostream& log_;
  
  void Filter(ChainSet* matches, int x, int y, char color, const vector<char>& clolr_map) const;
};

ChainDatabase::ChainDatabase(ostream& log)
    : pending_inputs_(100), pending_decisions_(100), log_(log) {
  vector<Field> chain_fields;
  ifstream fs(CHAIN_DATA_PATH);
  string line;
  int i = 0;
  while (getline(fs, line, '\n')) {
    assert(i < NUM_CHAIN_DATA);
    Field field(line);
    //cout << field.GetDebugOutput() << endl;
    chain_fields.push_back(field);
    for (int y = 1; y <= Field::HEIGHT; ++y) {
      for (int x = 1; x <= Field::WIDTH; ++x) {
        char color = field.Get(x, y);
        index_[ChainKey(x, y, color)][i] = true;
        //cout << x << "," << y << "," << int(field.Get(x, y)) << endl;
      }
    }
    ++i;
  }
}

void ChainDatabase::StartDecisionThread() {
  pthread_create(&decision_thread_, NULL, &DecisionThread, this);
}

void* ChainDatabase::DecisionThread(void* arg) {
  ChainDatabase* db = reinterpret_cast<ChainDatabase*>(arg);
  while (true) {
    Input input = db->pending_inputs_.dequeue();
    db->log_ << "DecisionThread begin: " << input.id << endl << flush;
    db->log_ << input.my_field.GetDebugOutput() << endl << flush;
    vector<Decision> decisions;
    db->Decide(input.my_field, &decisions);
    assert(decisions.size() == 2);
    db->pending_decisions_.enqueue(decisions[0]);
    db->log_ << "DecisionThread end: " << input.id << endl << flush;
  }
}

void ChainDatabase::Act(const Input& input) {
  if (input.id == 1 || input.state.i_chain_finished) {
    log_ << "enque: " << input.id << endl << flush;
    pending_inputs_.enqueue(input);
    //current_decision_.x = 2 + rand() % 4;
    //current_decision_.r = rand() % 4;
    current_decision_.x = 3;
    current_decision_.r = 0;
  } else {
    current_decision_.x = -1;
    current_decision_.r = -1;
  }
  Decision decision;
  while (pending_decisions_.try_dequeue(&decision)) {
    current_decision_ = decision;
  }
}

const bool LOOK_AT_NEXT_NEXT = true;
const bool USE_CACHE = true;

void ChainDatabase::Decide(
    const Field& field,
    vector<Decision>* decisions) const {
  vector<pair<char, char> > puyos_list;
  for (int i = 0; i < (LOOK_AT_NEXT_NEXT ? 3 : 2); ++i) {
    puyos_list.push_back(make_pair(field.GetNextPuyo(2 * i), field.GetNextPuyo(2 * i + 1)));
  }
  
  int max_matches = -1;
  if (LOOK_AT_NEXT_NEXT) {
    for (int cx = 1; cx <= Field::WIDTH; ++cx) {
      for (int cr = 0; cr < 4; ++cr) {
        Field field1 = field;
        //log_ << "cx = " << cx << ", cr = " << cr << endl << flush;
        if (!Put(&field1, Decision(cx, cr), puyos_list[0])) continue;
        for (int nx = 1; nx <= Field::WIDTH; ++nx) {
          for (int nr = 0; nr < 4; ++nr) {
            Field field2 = field1;
            if (!Put(&field2, Decision(nx, nr), puyos_list[1])) continue;
            for (int nnx = 1; nnx <= Field::WIDTH; ++nnx) {
              for (int nnr = 0; nnr < 4; ++nnr) {
                int this_num_matches = CountMatches(field2, puyos_list[2], Decision(nnx, nnr));
                if (this_num_matches > max_matches) {
                  max_matches = this_num_matches;
                  decisions->clear();
                  decisions->push_back(Decision(cx, cr));
                  decisions->push_back(Decision(nx, nr));
                  decisions->push_back(Decision(nnx, nnr));
                }
              }
            }
          }
        }
      }
    }
    
  } else {
    for (int cx = 1; cx <= Field::WIDTH; ++cx) {
      for (int cr = 0; cr < 4; ++cr) {
        Field field1 = field;
        if (!Put(&field1, Decision(cx, cr), puyos_list[0])) continue;
        for (int nx = 1; nx <= Field::WIDTH; ++nx) {
          for (int nr = 0; nr < 4; ++nr) {
            int this_num_matches = CountMatches(field1, puyos_list[1], Decision(nx, nr));
            if (this_num_matches > max_matches) {
              max_matches = this_num_matches;
              decisions->clear();
              decisions->push_back(Decision(cx, cr));
              decisions->push_back(Decision(nx, nr));
            }
          }
        }
      }
    }
    
  }
  assert(decisions->size() >= 2);
  log_ << "max_matches = " << max_matches << endl;
  Field max_field = field;
  //Put(&max_field, (*decisions)[0], puyos_list[0]);
  Put(&max_field, *decisions, puyos_list);
  log_ << "max_field:" << endl;
  log_ << max_field.GetDebugOutput() << endl;
}

int ChainDatabase::CountMatches(
    const Field& prev_field,
    const pair<char, char>& puyos,
    const Decision& decision) const {
  Field target = prev_field;
  int first_x = 0, first_y = 0, second_x = 0, second_y= 0;
  if (!Put(&target, decision, puyos, &first_x, &first_y, &second_x, &second_y)) {
    return 0;
  }
  log_ << target.GetDebugOutput() << endl << flush;

  Field temp_field = target;
  int chains, score, frames;
  temp_field.Simulate(&chains, &score, &frames);
  if (chains > 0 && chains < 5) return 0;

  Cache::iterator cache_it = cache_.find(prev_field);
  if (cache_it != cache_.end()) {
    //log_ << "cache hit" << endl << flush;
    vector<char> color_map;
    for (char i = NORMAL_COLOR_BEGIN; i < NORMAL_COLOR_END; ++i) {
      color_map.push_back(i);
    }
    ChainSet union_matches;
    CacheValue* new_cache_value = USE_CACHE ? &cache_[target] : NULL;
    if (new_cache_value) new_cache_value->perm_to_matches.clear();
    int perm_id = 0;
    do {
      ChainSet this_matches = cache_it->second.perm_to_matches.at(perm_id);
      Filter(&this_matches, first_x, first_y, puyos.first, color_map);
      Filter(&this_matches, second_x, second_y, puyos.second, color_map);
      if (new_cache_value) new_cache_value->perm_to_matches.push_back(this_matches);
      union_matches |= this_matches;
      ++perm_id;
    } while (next_permutation(color_map.begin(), color_map.end()));
    return union_matches.count();
    
  } else {
    //log_ << "no cache hit" << endl << flush;
    return CountMatches(target);
    
  }
}

int ChainDatabase::CountMatches(const Field& target) const {
  vector<char> color_map;
  for (char i = NORMAL_COLOR_BEGIN; i < NORMAL_COLOR_END; ++i) {
    color_map.push_back(i);
  }
  ChainSet union_matches;
  CacheValue* cache_value = USE_CACHE ? &cache_[target] : NULL;
  if (cache_value) cache_value->perm_to_matches.clear();
  do {
    ChainSet this_matches;
    GetMatches(target, color_map, &this_matches);
    if (cache_value) cache_value->perm_to_matches.push_back(this_matches);
    union_matches |= this_matches;
  } while (next_permutation(color_map.begin(), color_map.end()));
  /*
  for (set<int>::iterator it = union_matches.begin(); it != union_matches.end(); ++it) {
    log_ << *it << ",";
  }
  log_ << endl;
  */
  return union_matches.count();
}

void ChainDatabase::GetMatches(
    const Field& target, const vector<char>& color_map, ChainSet* matches) const {
  matches->set();
  for (int y = 1; y <= Field::HEIGHT; ++y) {
    for (int x = 1; x <= Field::WIDTH; ++x) {
      Filter(matches, x, y, target.Get(x, y), color_map);
    }
  }
  /*
  for (int i = 0; i < matches->size(); ++i) {
    log_ << (*matches)[i] << ",";
  }
  log_ << endl;
  */
}

void ChainDatabase::Filter(
    ChainSet* matches, int x, int y, char color, const vector<char>& color_map) const {
  if (color == EMPTY) return;
  map<ChainKey, ChainSet>::const_iterator it =
      index_.find(ChainKey(x, y, color_map.at(color - NORMAL_COLOR_BEGIN)));
  if (it == index_.end()) {
    matches->reset();
    return;
  }
  *matches &= it->second;
}

void Tokopuyo() {
  ChainDatabase db(cout);
  string color_sequence;
  for (int i = 0; i < 256; ++i) {
    color_sequence += static_cast<char>(
        '0' + NORMAL_COLOR_BEGIN + rand() % (NORMAL_COLOR_END - NORMAL_COLOR_BEGIN));
  }
  int seq_idx = 0;
  Field field;
  while (true) {
    string seq;
    for (int i = 0; i < 6; ++i) {
      seq += color_sequence[(seq_idx + i) % 256];
    }
    field.SetColorSequence(seq);
    seq_idx += 2;
    cout << field.GetDebugOutput() << endl;
    vector<Decision> decisions;
    db.Decide(field, &decisions);
    assert(!decisions.empty());
    Put(&field, decisions[0], make_pair(field.GetNextPuyo(0), field.GetNextPuyo(1)));
    string dummy;
    getline(cin, dummy, '\n');
  }
}

// argv[1] will have "Player1" for player 1, and "Player2" for player 2.
int main(int argc, char* argv[]) {
  // Logging.
  if (argc < 2) {
    return 1;
  }
  
  if (string(argv[1]) == "tokopuyo") {
    Tokopuyo();
    return 0;
  } else {
    string name = string(argv[1]) + ".txt";
    ofstream log(name.c_str());
    
    /*
    //Field field("000000000000000000000000000000000000000000000000700000445000457770556670477766");
    Field field("000000000000000000000000000000000000000000000000000000000000000000000000007767");
    int chains, score, frames;
    field.Simulate(&chains, &score, &frames);
    cout << chains << endl;
    cout << field.GetDebugOutput() << endl;
    return 0;
    */
    
    /*
    ChainDatabase tdb(cout);
    Field field("000000000000000000000000000000000000000000000000000000000000000000000000007767");
    //cout << tdb.CountMatches(field) << endl;
    cout << tdb.CountMatches(field, make_pair(5, 5), Decision(1, 2)) << endl;
    Put(&field, Decision(1, 2), make_pair(5, 5));
    cout << field.GetDebugOutput() << endl;
    cout << tdb.CountMatches(field) << endl;
    return 0;
    */

    ChainDatabase db(log);
    db.StartDecisionThread();
    
    // Make sure the CPU is connected to the duel server.
    // Echo back what we receive.
    // MAKE SURE to flush.
    Input input;
    GetInput(&input, log);
    cout << input.original << endl << flush;

    while (1) {
      GetInput(&input, log);
      //log << "Received: " << input.original << endl << flush;
      db.Act(input);
      if (db.current_decision().x >= 0) {
        cout << "ID=" << input.id
            << " X=" << db.current_decision().x << " R=" << db.current_decision().r << endl << flush;
        log << "Output: ID=" << input.id << " X=" << db.current_decision().x << " R=" << db.current_decision().r << endl << flush;
      }
    }
  }
}
