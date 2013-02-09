#ifndef CAPTURE_CAPTURE_H_
#define CAPTURE_CAPTURE_H_

#include <memory>
#include <string>

#include <SDL.h>

#include <core/state.h>
#include <core/field.h>
#include <duel/screen.h>

using namespace std;

class Commentator;
class PuyoFu;

class Capture {
 public:
  enum Mode {
    NONE,
    VCA,
    NICO,
  };

  struct HSV {
    float h, s, v;
  };

  struct RGB {
    RGB() : r(0), g(0), b(0) {}
    RGB(float r, float g, float b) : r(r), g(g), b(b) {}

    void add(RGB c) {
      r += c.r;
      g += c.g;
      b += c.b;
    }

    void div(float f) {
      if (f) {
        r /= f;
        g /= f;
        b /= f;
      } else {
        r = g = b = 0;
      }
    }

    HSV toHSV() const;
    float r, g, b;
  };

  enum RealColor {
    RC_EMPTY = EMPTY,
    RC_OJAMA = OJAMA,
    RC_RED = RED,
    RC_BLUE = BLUE,
    RC_YELLOW = YELLOW,
    RC_GREEN = GREEN,
    RC_PURPLE,
    RC_VANISHING,
    RC_END
  };

  enum GameState {
    GAME_INIT,
    GAME_TITLE,
    GAME_FINISHED,
    GAME_MODE_SELECT,
    GAME_RUNNING,
  };

  explicit Capture(SDL_Surface* scr);
  Capture(int w, int h, int bpp);

  void addFrame(SDL_Surface* surf);

  void show();

  void save(const char* bmp_filename);

  unsigned long long capture_ticks() const { return capture_ticks_; }
  int capture_frames() const { return capture_frames_; }

  int player_index(int i) const {
    return capture_frames_ % 2 * 2 + i;
  }

  int prev_player_index(int i) const {
    return 2 - capture_frames_ % 2 * 2 + i;
  }

  string getMessageFor(int pi);

  GameState game_state() const { return game_state_; }

  Colors getColor(int i, int x, int y) const;
  Colors getNext(int i, int j) const;
  RealColor getRealColor(int i, int x, int y) const;
  bool isVanishing(int i, int x, int y) const;
  int numVanishing() const;
  unsigned int getState(int i) const {
    return state_[i];
  }
  void setState(int i, unsigned int s);

  void setFrameInfo(string frame_info) {
    frame_info_ = frame_info;
  }

  void updateAIField(int i);
  void updateAINext(int i);

  void setAIMessage(int pi, const string& msg);

  Screen* getScreen() { return scr_.get(); }

 private:
  struct Box : public Screen::Box {};

  void init();

  bool detectMode();
  void calcState();
  Colors getAIColor(RealColor rc);
  void setAIColor(RealColor rc, Colors c);
  bool isTitle() const;
  bool isModeSelect() const;
  void updateWinner();
  void finishGame();
  void setGrounded(int i);
  float calcColorDiff2(int i, int x, int y) const;
  float calcNextColorDiff2(int i, int x) const;
  bool isStable(int i, int x, int y) const;
  bool isNextStable(int i, int x) const;
  void handleWnextArrival(int i);
  int countNumVanishing(int i, int x, int y);
  bool isStableField(int i) const;
  void moveNexts(int i);

  void dumpStateInfo() const;

  void maybeUpdateComment();

  RealColor toPuyo(RGB rgb, HSV hsv, int x, int y);
  RealColor toPuyo(RGB rgb, int x, int y);

  auto_ptr<Screen> scr_;
  SDL_Surface* surf_;
  Mode mode_;
  Box bb_[2][6][14];
  RGB rgb_[4][6][13];
  RealColor puyo_[4][6][13];
  char is_vanishing_[4][6][13];
  int num_vanishing_[2];
  int num_ojama_[2];

  unsigned int state_[2];
  bool has_next_[2];
  bool has_wnext_[2];
  int frames_after_next_disappeared_[2];
  bool grounded_[2];
  bool chain_finished_[2];
  GameState game_state_;
  int winner_;

  // Translation table from RealColor to Colors.
  Colors color_map_[6];
  // The field for AI.
  Colors ai_puyo_[2][6][13];
  Colors ai_next_[2][6];
  int ai_num_ojama_[2];
  string ai_msg_[2];

  unsigned long long capture_ticks_;
  int capture_frames_;

  string frame_info_;

  auto_ptr<Commentator> commentator_;
  auto_ptr<PuyoFu> puyo_fu_;
};

#endif  // CAPTURE_CAPTURE_H_
