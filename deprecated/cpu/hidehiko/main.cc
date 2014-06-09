#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <glog/logging.h>
#include <glog/log_severity.h>
#include <vector>

using namespace std;
using namespace google;

class Decision {
 public:
  Decision(int x, int r) : x_(x), r_(r) {
  }

  bool IsValid() const { return x_ > 0 && r_ > 0; }

  int x() const { return x_; }
  int r() const { return r_; }

  static const Decision VALUES[];
 private:
  int x_;
  int r_;
};

const Decision Decision::VALUES[] = {
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

enum Color {
  EMPTY = 0,
  OJAMA = 1,
  WALL = 2,
  RED = 4,
  BLUE = 5,
  YELLOW = 6,
  GREEN = 7,
};

class Field {
 public:
  static const int WIDTH = 6;
  static const int HEIGHT = 12;

  // incl. sentinel and top space.
  static const int MAP_WIDTH = 1 + WIDTH + 1;
  static const int MAP_HEIGHT = 1 + HEIGHT + 3;

  Field() {
    Clear();
  }

  void Clear() {
    memset(field_, EMPTY, MAP_WIDTH * MAP_HEIGHT);
    for (int i = 0; i < MAP_WIDTH; ++i) {
      field_[i][0] = WALL;
      field_[i][MAP_HEIGHT - 1] = WALL;
    }
    for (int i = 1; i < MAP_HEIGHT - 1; ++i) {
      field_[0][i] = WALL;
      field_[MAP_WIDTH - 1][i] = WALL;
    }
  }

  void ParseFrom(const string& field) {
    int height = (field.size() == WIDTH * HEIGHT) ? HEIGHT : (HEIGHT + 1);
    int index = 0;
    for (int i = 1; i <= WIDTH; ++i) {
      for (int j = 1; j < height; ++j) {
        field_[i][j] = static_cast<unsigned char>(field[index++] - '0');
      }
    }
  }

  bool IsAvailableDecision(const Decision& decision) const {
    int x1 = decision.x();
    int x2 = x1 + (decision.r() == 3) - (decision.r() == 1);
    return field_[x1][HEIGHT] == EMPTY && field_[x2][HEIGHT] == EMPTY;
  }

  void Put(const Decision& decision,
           unsigned char color1, unsigned char color2) {
    DCHECK(IsAvailableDecision(decision));
    int x1 = decision.x();
    int x2 = x1 + (decision.r() == 3) - (decision.r() == 1);
    field_[x1][HEIGHT] = color1;
    field_[x2][HEIGHT] = color2;
  }

  void Drop() {
    for (int i = 1; i <= WIDTH; ++i) {
      int k = 1;
      for (int j = 1; j <= HEIGHT + 2; ++j) {
        if (field_[i][j] != EMPTY) {
          field_[i][k] = field_[i][j];
          ++k;
        }
      }
      for (; k <= HEIGHT + 2; ++k) {
        field_[i][k] = EMPTY;
      }
    }
  }

  bool Vanish() {
    bool result = false;
    while (true) {
      bool erased = false;
      bool erase_check[MAP_WIDTH][MAP_HEIGHT] = {};
      for (int i = 1; i <= WIDTH; ++i) {
        for (int j = 1; j <= HEIGHT; ++j) {
          if (erase_check[i][j]) {
            continue;
          }
          if (field_[i][j] == EMPTY) {
            erase_check[i][j] = true;
            continue;
          }
          int count = 0;
          VanishInternal(i, j, field_[i][j], &count, erase_check);
          if (count >= 4) {
            Erase(i, j, field_[i][j]);
            erased = true;
          }
        }
      }
      if (!erased) {
        break;
      }
      result = true;
      Drop();
    }
    return result;
  }

  static const int VANISH_CHECKED = 1;

 private:
  void VanishInternal(int i, int j, int color,
                      int* count, bool erase_check[MAP_WIDTH][MAP_HEIGHT]) {
    if (erase_check[i][j]) {
      return;
    }
    erase_check[i][j] = true;
    if (field_[i][j] != color) {
      return;
    }

    ++*count;
    VanishInternal(i + 1, j, color, count, erase_check);
    VanishInternal(i - 1, j, color, count, erase_check);
    VanishInternal(i, j + 1, color, count, erase_check);
    VanishInternal(i, j - 1, color, count, erase_check);
  }

  void Erase(int i, int j, int color) {
    if (field_[i][j] == color) {
      field_[i][j] = EMPTY;
      EraseOjama(i + 1, j);
      EraseOjama(i - 1, j);
      EraseOjama(i, j + 1);
      EraseOjama(i, j - 1);
    }
  }

  void EraseOjama(int i, int j) {
    if (field_[i][j] == OJAMA) {
      field_[i][j] = EMPTY;
    }
  }

  unsigned char field_[MAP_WIDTH][MAP_HEIGHT];
};

class ControlPuyo {
 public:
  ControlPuyo() {
  }
  ControlPuyo(int x, int y, int r) : x_(x), y_(y), r_(r) {
  }

  int x() const { return x_; }
  int y() const { return y_; }
  int r() const { return r_; }

  void set_x(int x) { x_ = x; }
  void set_y(int y) { y_ = y; }
  void set_r(int r) { r_ = r; }

 private:
  int x_;
  int y_;
  int r_;
};

class Player {
 public:
  Player() {
  }

  Field* mutable_field() { return &field_; }
  ControlPuyo* mutable_puyo() { return &puyo_; }
  void set_notice(const unsigned char* notice) {
    memcpy(notice_, notice, 6);
  }
  void set_score(int score) { score_ = score; }
  void set_ojama(int ojama) { ojama_ = ojama; }

  const Field& field() const { return field_; }
  const ControlPuyo& puyo() const { return puyo_; }
  const unsigned char* notice() const { return notice_; }
  int score() const { return score_; }
  int ojama() const { return ojama_; }

 private:
  Field field_;
  ControlPuyo puyo_;
  unsigned char notice_[6];
  int score_;
  int ojama_;
};

class Input {
 public:
  Input() {
  }

  void ParseFrom(const string& line) {
    istringstream is(line);
    string field;
    while (getline(is, field, ' ')) {
      if (field.compare(0, 3, "ID=", 0, 3) == 0) {
        istringstream value(field.c_str() + 3);
        value >> id_;
        continue;
      }
      if (field.compare(0, 6, "STATE=", 0, 6) == 0) {
        istringstream value(field.c_str() + 6);
        value >> state_;
        continue;
      }

      // Just ignore ACK/NACK for now.
      if (field.compare(0, 4, "ACK=", 0, 4) == 0 ||
          field.compare(0, 5, "NACK=", 0, 5) == 0) {
        continue;
      }

      Player* player = (field[0] == 'Y') ? &player1_ : &player2_;
      switch (field[1]) {
        case 'F':
          player->mutable_field()->ParseFrom(field.substr(3));
          break;
        case 'P':
          player->set_notice(
              reinterpret_cast<const unsigned char*>(field.data() + 6));
          break;
        case 'X': {
          istringstream value(field.c_str() + 3);
          int x;
          value >> x;
          player->mutable_puyo()->set_x(x);
          break;
        }
        case 'Y': {
          istringstream value(field.c_str() + 3);
          int y;
          value >> y;
          player->mutable_puyo()->set_y(y);
          break;
        }
        case 'R': {
          istringstream value(field.c_str() + 3);
          int r;
          value >> r;
          player->mutable_puyo()->set_r(r);
          break;
        }
        case 'O': {
          istringstream value(field.c_str() + 3);
          int ojama;
          value >> ojama;
          player->set_ojama(ojama);
          break;
        }
        case 'S': {
          istringstream value(field.c_str() + 3);
          int score;
          value >> score;
          player->set_score(score);
          break;
        }
        default:
          LOG(FATAL) << "Unexpected field: " << field;
      }
    }
  }

  int id() const { return id_; }
  int state() const { return state_; }
  const Player& player1() const { return player1_; }
  const Player& player2() const { return player2_; }

 private:
  int id_;
  int state_;
  Player player1_;
  Player player2_;
};

class Cpu {
 public:
  Cpu() : count_(0), pending_(false) {
  }

  Decision Evaluate(const Input& input) {
    if ((input.state() & (1 << 4))) {
      pending_ = false;
    }
    if (pending_) {
      LOG(INFO) << "pending" << count_;
      return Decision(0, 0);
    }

    if (!(input.state() & (1 << 0))) {
      LOG(INFO) << "unavailable" << count_;
      return Decision(0, 0);
    }

    LOG(INFO) << count_;
    ++count_;
    pending_ = true;
    return (count_ & 1) ? Decision(1, 1) : Decision(5, 1);
  }
 private:
  int count_;
  bool pending_;
};

class Cpu2 {
 public:
  Cpu2() : pending_(false), count_(0) {
  }

  Decision Evaluate(const Input& input) {
    if ((input.state() & (1 << 4))) {
      pending_ = false;
    }

    if (pending_) {
      return Decision(0, 0);
    }

    if (!(input.state() & (1 << 0))) {
      return Decision(0, 0);
    }

    LOG(ERROR) << "HOGE" << count_;
    ++count_;
    vector<int> available_index;
    for (int i = 0; i < sizeof(Decision::VALUES) / sizeof(Decision); ++i) {
      const Decision& decision = Decision::VALUES[i];
      if (!input.player1().field().IsAvailableDecision(decision)) {
        continue;
      }
      Field field(input.player1().field());
      field.Put(decision, input.player1().notice()[0], input.player1().notice()[1]);
      field.Drop();
      if (field.Vanish()) {
        pending_ = true;
        LOG(ERROR) << decision.x() << " " << decision.r();
        return decision;
      }
      available_index.push_back(i);
    }

    if (available_index.empty()) {
      return Decision(0, 0);
    }
    // TODO
    pending_ = true;
    const Decision& d =
        Decision::VALUES[available_index[rand() % available_index.size()]];
    LOG(ERROR) << d.x() << ", " << d.r();
    return d;
  }

 private:
  int count_;
  bool pending_;
};

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  LOG(ERROR) << "hidehiko CPU " << argv[1];
  FlushLogFiles(ERROR);

  string line;
  getline(cin, line);
  cout << line << endl << flush;

  Cpu2 cpu;
  Input input;
  while (getline(cin, line)) {
    LOG(INFO) << line;
    input.ParseFrom(line);
    Decision decision = cpu.Evaluate(input);
    if (decision.IsValid()) {
      LOG(INFO) << "(x, r) = " << decision.x() << ", " << decision.r();
      cout << "ID=" << input.id()
           << " X=" << decision.x()
           << " R=" << decision.r() << endl << flush;
    } else {
      LOG(INFO) << "(x, r) = (0, 0)";
      cout << "ID=" << input.id() << endl << flush;
    }
  }

  return 0;
}
