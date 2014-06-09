#ifndef CAPTURE_CAPTURE_H_
#define CAPTURE_CAPTURE_H_

#include <memory>
#include <string>

#include <SDL.h>

#include "base/base.h"
#include "core/state.h"
#include "core/deprecated_field.h"
#include "gui/drawer.h"
#include "gui/screen.h"
#include "duel/game_state_observer.h"

using namespace std;

class Commentator;
class PuyoFu;

class Capture : public Drawer {
public:
    enum Mode {
        NONE,
        VCA,
        NICO,
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


    Capture();
    virtual ~Capture();

    void addSource(Source* source);
    void start();
    void stop();

    // drawer interface
    virtual void onInit(Screen*) OVERRIDE;
    virtual void draw(Screen*) const OVERRIDE;

    void addSourceFrame(UniqueSDLSurface surface);

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

private:
    struct Box : public Screen::Box {};

    void init();

    bool detectMode();
    void calcState();
    Colors getAIColor(RealColor rc, bool will_allocate = true);
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

    UniqueSDLSurface sourceSurface_;
    std::vector<GameStateObserver*> observers_;

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
    int start_animation_frames_;
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

    std::unique_ptr<PuyoFu> puyo_fu_;
};

#endif  // CAPTURE_CAPTURE_H_
