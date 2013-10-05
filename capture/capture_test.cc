#include <stdio.h>
#include <stdlib.h>

#include <memory>
#include <vector>

#include <gflags/gflags.h>
#include <gtest/gtest.h>

#include <SDL.h>
#include <SDL_image.h>

#define private public
#include "capture.h"
#undef private

using namespace std;

DECLARE_bool(commentator);

class CaptureTest : public testing::Test {
 protected:
  ~CaptureTest() {
    for (size_t i = 0; i < surfaces_.size(); i++) {
      SDL_FreeSurface(surfaces_[i]);
    }
  }

  Capture* createCapture(const char* img_filename) {
    SDL_Surface* s = createSurface(img_filename);

    Capture* cap = new Capture(s);
    cap->addFrame(s);
    return cap;
  }

  void addFrame(Capture* cap, const char* img_filename) {
    SDL_Surface* s = createSurface(img_filename);
    cap->addFrame(s);
  }

 private:
  SDL_Surface* createSurface(const char* img_filename) {
    SDL_Surface* s = IMG_Load(img_filename);
    if (!s) {
      fprintf(stderr, "Failed to load %s\n", img_filename);
      abort();
    }
    surfaces_.push_back(s);
    return s;
  }

  vector<SDL_Surface*> surfaces_;
};

TEST_F(CaptureTest, mode_select_nico) {
  auto_ptr<Capture> cap(createCapture("test/mode_select_nico.jpg"));
  EXPECT_EQ(Capture::GAME_MODE_SELECT, cap->game_state());
  cap.reset(createCapture("test/mode_select_nico2.gif"));
  EXPECT_EQ(Capture::GAME_MODE_SELECT, cap->game_state());
}

TEST_F(CaptureTest, mode_select_vca) {
  auto_ptr<Capture> cap(createCapture("test/mode_select_vca.jpg"));
  EXPECT_EQ(Capture::GAME_MODE_SELECT, cap->game_state());
}

TEST_F(CaptureTest, title_nico) {
  auto_ptr<Capture> cap(createCapture("test/title_nico0.jpg"));
  EXPECT_EQ(Capture::GAME_TITLE, cap->game_state());
  cap.reset(createCapture("test/title_nico1.jpg"));
  EXPECT_EQ(Capture::GAME_TITLE, cap->game_state());
  cap.reset(createCapture("test/title_nico2.jpg"));
  EXPECT_EQ(Capture::GAME_TITLE, cap->game_state());
}

TEST_F(CaptureTest, running_vca) {
  auto_ptr<Capture> cap(createCapture("test/running_vca.jpg"));
  EXPECT_EQ(Capture::GAME_INIT, cap->game_state());
}

TEST_F(CaptureTest, vanish_vca) {
  auto_ptr<Capture> cap;
  // The color for AI depends on the color map. Let's just make sure
  // the vanishing puyos have the same color.
  Colors c;

  cap.reset(new Capture(NULL));
  cap->setState(0, STATE_YOU_CAN_PLAY);
  addFrame(cap.get(), "test/vanish_vca_y.gif");
  cap->updateAIField(0);
  addFrame(cap.get(), "test/vanish_vca_y.gif");
  EXPECT_TRUE(cap->isVanishing(0, 2, 11));
  EXPECT_EQ(Capture::RC_YELLOW, cap->getRealColor(0, 2, 11));
  EXPECT_TRUE(cap->isVanishing(0, 3, 11));
  EXPECT_EQ(Capture::RC_YELLOW, cap->getRealColor(0, 3, 11));
  EXPECT_TRUE(cap->isVanishing(0, 3, 10));
  EXPECT_EQ(Capture::RC_YELLOW, cap->getRealColor(0, 3, 10));
  EXPECT_TRUE(cap->isVanishing(0, 3, 9));
  EXPECT_EQ(Capture::RC_YELLOW, cap->getRealColor(0, 3, 9));
  EXPECT_EQ(4, cap->numVanishing());
  EXPECT_EQ(STATE_YOU_GROUNDED, cap->getState(0) & STATE_YOU_GROUNDED);
  c = cap->getColor(0, 2, 11);
  EXPECT_EQ(c, cap->getColor(0, 3, 11));
  EXPECT_EQ(c, cap->getColor(0, 3, 10));
  EXPECT_EQ(c, cap->getColor(0, 3, 9));

  cap.reset(new Capture(NULL));
  cap->setState(0, STATE_YOU_CAN_PLAY);
  addFrame(cap.get(), "test/vanish_vca_r.gif");
  cap->updateAIField(0);
  addFrame(cap.get(), "test/vanish_vca_r.gif");
  EXPECT_TRUE(cap->isVanishing(0, 2, 11));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(0, 2, 11));
  EXPECT_TRUE(cap->isVanishing(0, 2, 10));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(0, 2, 10));
  EXPECT_TRUE(cap->isVanishing(0, 3, 10));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(0, 3, 10));
  EXPECT_TRUE(cap->isVanishing(0, 3, 9));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(0, 3, 9));
  EXPECT_EQ(4, cap->numVanishing());
  EXPECT_EQ(STATE_YOU_GROUNDED, cap->getState(0) & STATE_YOU_GROUNDED);
  c = cap->getColor(0, 2, 11);
  EXPECT_EQ(c, cap->getColor(0, 2, 10));
  EXPECT_EQ(c, cap->getColor(0, 3, 10));
  EXPECT_EQ(c, cap->getColor(0, 3, 9));

  cap.reset(new Capture(NULL));
  cap->setState(0, STATE_YOU_CAN_PLAY);
  addFrame(cap.get(), "test/vanish_vca_p.gif");
  cap->updateAIField(0);
  addFrame(cap.get(), "test/vanish_vca_p.gif");
  EXPECT_TRUE(cap->isVanishing(0, 0, 11));
  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(0, 0, 11));
  EXPECT_TRUE(cap->isVanishing(0, 1, 11));
  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(0, 1, 11));
  EXPECT_TRUE(cap->isVanishing(0, 2, 11));
  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(0, 2, 11));
  EXPECT_TRUE(cap->isVanishing(0, 2, 10));
  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(0, 2, 10));
  EXPECT_EQ(4, cap->numVanishing());
  EXPECT_EQ(STATE_YOU_GROUNDED, cap->getState(0) & STATE_YOU_GROUNDED);
  c = cap->getColor(0, 0, 11);
  EXPECT_EQ(c, cap->getColor(0, 1, 11));
  EXPECT_EQ(c, cap->getColor(0, 2, 11));
  EXPECT_EQ(c, cap->getColor(0, 2, 10));

  cap.reset(new Capture(NULL));
  cap->setState(0, STATE_YOU_CAN_PLAY);
  addFrame(cap.get(), "test/vanish_vca_g.gif");
  cap->updateAIField(0);
  addFrame(cap.get(), "test/vanish_vca_g.gif");
  EXPECT_TRUE(cap->isVanishing(0, 2, 11));
  EXPECT_EQ(Capture::RC_GREEN, cap->getRealColor(0, 2, 11));
  EXPECT_TRUE(cap->isVanishing(0, 3, 11));
  EXPECT_EQ(Capture::RC_GREEN, cap->getRealColor(0, 3, 11));
  EXPECT_TRUE(cap->isVanishing(0, 3, 10));
  EXPECT_EQ(Capture::RC_GREEN, cap->getRealColor(0, 3, 10));
  EXPECT_TRUE(cap->isVanishing(0, 4, 10));
  EXPECT_EQ(Capture::RC_GREEN, cap->getRealColor(0, 4, 10));
  EXPECT_EQ(4, cap->numVanishing());
  EXPECT_EQ(STATE_YOU_GROUNDED, cap->getState(0) & STATE_YOU_GROUNDED);
  c = cap->getColor(0, 2, 11);
  EXPECT_EQ(c, cap->getColor(0, 3, 11));
  EXPECT_EQ(c, cap->getColor(0, 3, 10));
  EXPECT_EQ(c, cap->getColor(0, 4, 10));

  cap.reset(new Capture(NULL));
  cap->setState(0, STATE_YOU_CAN_PLAY);
  addFrame(cap.get(), "test/vanish_vca_b.gif");
  cap->updateAIField(0);
  addFrame(cap.get(), "test/vanish_vca_b.gif");
  EXPECT_TRUE(cap->isVanishing(0, 2, 9));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(0, 2, 9));
  EXPECT_TRUE(cap->isVanishing(0, 3, 10));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(0, 3, 10));
  EXPECT_TRUE(cap->isVanishing(0, 3, 9));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(0, 3, 9));
  EXPECT_TRUE(cap->isVanishing(0, 3, 8));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(0, 3, 8));
  EXPECT_TRUE(cap->isVanishing(0, 1, 9));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 1, 9));
  EXPECT_EQ(5, cap->numVanishing());
  EXPECT_EQ(STATE_YOU_GROUNDED, cap->getState(0) & STATE_YOU_GROUNDED);
  c = cap->getColor(0, 2, 9);
  EXPECT_EQ(c, cap->getColor(0, 3, 10));
  EXPECT_EQ(c, cap->getColor(0, 3, 9));
  EXPECT_EQ(c, cap->getColor(0, 3, 8));
  EXPECT_EQ(OJAMA, cap->getColor(0, 1, 9));

  // We shouldn't send STATE_YOU_GROUNDED again.
  addFrame(cap.get(), "test/vanish_vca_b.gif");
  EXPECT_NE(STATE_YOU_GROUNDED, cap->getState(0) & STATE_YOU_GROUNDED);
}

TEST_F(CaptureTest, vanish_vca_middle) {
  auto_ptr<Capture> cap(new Capture(NULL));
  cap->setState(1, STATE_YOU_CAN_PLAY);
  addFrame(cap.get(), "test/vanish_vca_middle.gif");
  cap->updateAIField(1);
  addFrame(cap.get(), "test/vanish_vca_middle.gif");
  EXPECT_TRUE(cap->isVanishing(1, 2, 6));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(1, 2, 6));
  EXPECT_TRUE(cap->isVanishing(1, 2, 7));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(1, 2, 7));
  EXPECT_TRUE(cap->isVanishing(1, 3, 6));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(1, 3, 6));
  EXPECT_TRUE(cap->isVanishing(1, 4, 6));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(1, 4, 6));
  EXPECT_TRUE(cap->isVanishing(1, 4, 7));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(1, 4, 7));
  EXPECT_EQ(5, cap->numVanishing());
  EXPECT_EQ(STATE_YOU_GROUNDED, cap->getState(1) & STATE_YOU_GROUNDED);
  Colors c = cap->getColor(1, 2, 6);
  EXPECT_EQ(c, cap->getColor(1, 2, 7));
  EXPECT_EQ(c, cap->getColor(1, 3, 6));
  EXPECT_EQ(c, cap->getColor(1, 4, 6));
  EXPECT_EQ(c, cap->getColor(1, 4, 7));
}

TEST_F(CaptureTest, wnext_vca_b1) {
  auto_ptr<Capture> cap(createCapture("test/wnext_vca_b1.gif"));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(1, 3, 12));

  // Check other nexts for sure.
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(0, 0, 12));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(0, 1, 12));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(0, 2, 12));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(0, 3, 12));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(1, 0, 12));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(1, 1, 12));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(1, 2, 12));
}

TEST_F(CaptureTest, wnext_vca_b2) {
  auto_ptr<Capture> cap(createCapture("test/wnext_vca_b2.gif"));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(0, 3, 12));

  // Check other nexts for sure.
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(0, 0, 12));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(0, 1, 12));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(0, 2, 12));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(1, 0, 12));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(1, 1, 12));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(1, 2, 12));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(1, 3, 12));
}

TEST_F(CaptureTest, wnext_vca_r1) {
  auto_ptr<Capture> cap(createCapture("test/wnext_vca_r1.gif"));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(0, 2, 12));

  // Check other nexts for sure.
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(0, 0, 12));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(0, 1, 12));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(0, 3, 12));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(1, 0, 12));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(1, 1, 12));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(1, 2, 12));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(1, 3, 12));
}

TEST_F(CaptureTest, wnext_vca_r2) {
  auto_ptr<Capture> cap(createCapture("test/wnext_vca_r2.gif"));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(0, 2, 12));

  // Check other nexts for sure.
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(0, 0, 12));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(0, 1, 12));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(0, 3, 12));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(1, 0, 12));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(1, 1, 12));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(1, 2, 12));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(1, 3, 12));
}

TEST_F(CaptureTest, wnext_vca_empty_1p) {
  auto_ptr<Capture> cap(createCapture("test/wnext_vca_empty_1p.gif"));
  EXPECT_EQ(Capture::RC_EMPTY, cap->getRealColor(0, 2, 12));
  EXPECT_EQ(Capture::RC_EMPTY, cap->getRealColor(0, 3, 12));
}

TEST_F(CaptureTest, wnext_vca_empty_2p) {
  auto_ptr<Capture> cap(createCapture("test/wnext_vca_empty_2p.bmp"));
  EXPECT_EQ(Capture::RC_EMPTY, cap->getRealColor(1, 2, 12));
  EXPECT_EQ(Capture::RC_EMPTY, cap->getRealColor(1, 3, 12));
}

TEST_F(CaptureTest, wnext_vca_y2u) {
  auto_ptr<Capture> cap(createCapture("test/wnext_vca_y2u.gif"));
  EXPECT_EQ(Capture::RC_YELLOW, cap->getRealColor(1, 2, 12));
  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(1, 3, 12));
}

TEST_F(CaptureTest, wnext_vca_y2d) {
  auto_ptr<Capture> cap(createCapture("test/wnext_vca_y2d.gif"));
  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(1, 2, 12));
  EXPECT_EQ(Capture::RC_YELLOW, cap->getRealColor(1, 3, 12));
}

TEST_F(CaptureTest, wnext_nico_p1) {
  auto_ptr<Capture> cap(createCapture("test/wnext_nico_p1.gif"));
  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(0, 2, 12));
  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(0, 3, 12));
  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(1, 2, 12));
  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(1, 3, 12));

  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(0, 0, 12));
  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(0, 1, 12));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(1, 0, 12));
  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(1, 1, 12));

  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(0, 2, 11));
  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(0, 2, 10));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(1, 2, 11));
  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(1, 2, 10));
}

// Tests for weeping reds.
TEST_F(CaptureTest, anime_vca_r1) {
  auto_ptr<Capture> cap(createCapture("test/anime_vca_r1.gif"));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(0, 2, 12));
}
TEST_F(CaptureTest, anime_vca_r2) {
  auto_ptr<Capture> cap(createCapture("test/anime_vca_r2.gif"));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(0, 3, 10));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(1, 0, 11));
}

TEST_F(CaptureTest, empty_wnext_vca) {
  auto_ptr<Capture> cap(createCapture("test/empty_wnext_vca.gif"));
  EXPECT_EQ(Capture::RC_EMPTY, cap->getRealColor(0, 2, 12));
  EXPECT_EQ(Capture::RC_EMPTY, cap->getRealColor(0, 3, 12));
  EXPECT_EQ(Capture::RC_EMPTY, cap->getRealColor(1, 2, 12));
  EXPECT_EQ(Capture::RC_EMPTY, cap->getRealColor(1, 3, 12));
}

TEST_F(CaptureTest, next_arrival_vca) {
  auto_ptr<Capture> cap(createCapture("test/next_arrival_vca1.gif"));
  EXPECT_FALSE(cap->has_next_[0]);
  EXPECT_FALSE(cap->has_next_[1]);
  addFrame(cap.get(), "test/next_arrival_vca2.gif");
  EXPECT_FALSE(cap->has_next_[0]);
  EXPECT_FALSE(cap->has_next_[1]);
  addFrame(cap.get(), "test/next_arrival_vca3.gif");
  EXPECT_FALSE(cap->has_next_[0]);
  EXPECT_FALSE(cap->has_next_[1]);
  // The animation makes the difference.
  // TODO(hamaji): Check if we can minimize this gap.
  addFrame(cap.get(), "test/next_arrival_vca4.gif");
  EXPECT_FALSE(cap->has_next_[0]);
  EXPECT_TRUE(cap->has_next_[1]);
  addFrame(cap.get(), "test/next_arrival_vca5.gif");
  EXPECT_FALSE(cap->has_next_[0]);
  EXPECT_TRUE(cap->has_next_[1]);
  addFrame(cap.get(), "test/next_arrival_vca6.gif");
  EXPECT_TRUE(cap->has_next_[0]);
  EXPECT_TRUE(cap->has_next_[1]);
}

TEST_F(CaptureTest, wnext_arrival_vca) {
  auto_ptr<Capture> cap(createCapture("test/wnext_arrival_vca1.gif"));
  EXPECT_FALSE(cap->has_wnext_[0]);
  addFrame(cap.get(), "test/wnext_arrival_vca2.gif");
  EXPECT_FALSE(cap->has_wnext_[0]);
  addFrame(cap.get(), "test/wnext_arrival_vca3.gif");
  EXPECT_TRUE(cap->has_wnext_[0]);
}

TEST_F(CaptureTest, moving_vca) {
  auto_ptr<Capture> cap(createCapture("test/moving_vca.gif"));
  EXPECT_EQ(0, cap->numVanishing());
}

TEST_F(CaptureTest, gameover_vca) {
  auto_ptr<Capture> cap(createCapture("test/gameover_vca_1p.gif"));
  addFrame(cap.get(), "test/gameover_vca_1p.gif");
  EXPECT_EQ(Capture::GAME_FINISHED, cap->game_state());
  cap.reset(createCapture("test/gameover_vca_2p.gif"));
  addFrame(cap.get(), "test/gameover_vca_2p.gif");
  EXPECT_EQ(Capture::GAME_FINISHED, cap->game_state());
}

TEST_F(CaptureTest, ojama_dropped_vca) {
  auto_ptr<Capture> cap(createCapture("test/ojama_dropped_vca.gif"));
  ASSERT_EQ(0, cap->getState(0));
  cap->setState(0, STATE_YOU_CAN_PLAY);
  addFrame(cap.get(), "test/ojama_dropped_vca.gif");
  EXPECT_EQ(STATE_YOU_GROUNDED, cap->getState(0) & STATE_YOU_GROUNDED);
  EXPECT_EQ(0, cap->getState(0) & STATE_YOU_CAN_PLAY);
}

TEST_F(CaptureTest, ojama_puyos_nico) {
  auto_ptr<Capture> cap(createCapture("test/puyos_nico1.png"));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(0, 0, 5));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(0, 0, 7));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(0, 0, 8));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(0, 0, 9));

  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(0, 3, 2));
  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(0, 3, 3));
  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(0, 3, 4));
  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(0, 3, 6));

  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(0, 0, 4));
  EXPECT_EQ(Capture::RC_YELLOW, cap->getRealColor(0, 0, 6));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(0, 0, 10));
  EXPECT_EQ(Capture::RC_YELLOW, cap->getRealColor(0, 0, 11));

  EXPECT_EQ(Capture::RC_YELLOW, cap->getRealColor(0, 0, 12));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(0, 1, 12));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(0, 2, 12));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(0, 3, 12));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(1, 0, 12));
  EXPECT_EQ(Capture::RC_YELLOW, cap->getRealColor(1, 1, 12));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(1, 2, 12));
  EXPECT_EQ(Capture::RC_YELLOW, cap->getRealColor(1, 3, 12));

  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(1, 0, 0));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(1, 3, 0));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(1, 4, 0));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(1, 5, 0));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(1, 4, 1));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(1, 5, 1));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(1, 1, 4));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(1, 1, 5));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(1, 2, 6));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(1, 2, 7));

  cap.reset(createCapture("test/puyos_nico2.png"));
  EXPECT_EQ(Capture::RC_YELLOW, cap->getRealColor(1, 0, 4));
  EXPECT_EQ(Capture::RC_YELLOW, cap->getRealColor(1, 0, 5));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(1, 0, 6));
  EXPECT_EQ(Capture::RC_YELLOW, cap->getRealColor(1, 0, 7));
  EXPECT_EQ(Capture::RC_YELLOW, cap->getRealColor(1, 0, 8));
  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(1, 0, 9));
  EXPECT_EQ(Capture::RC_YELLOW, cap->getRealColor(1, 0, 10));
  EXPECT_EQ(Capture::RC_YELLOW, cap->getRealColor(1, 0, 11));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(1, 1, 8));
  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(1, 1, 9));
  EXPECT_EQ(Capture::RC_YELLOW, cap->getRealColor(1, 1, 10));
  EXPECT_EQ(Capture::RC_BLUE, cap->getRealColor(1, 1, 11));

  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(0, 0, 12));
  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(0, 1, 12));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(0, 2, 12));
  EXPECT_EQ(Capture::RC_YELLOW, cap->getRealColor(0, 3, 12));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(1, 0, 12));
  EXPECT_EQ(Capture::RC_PURPLE, cap->getRealColor(1, 1, 12));
  EXPECT_EQ(Capture::RC_RED, cap->getRealColor(1, 2, 12));
  EXPECT_EQ(Capture::RC_YELLOW, cap->getRealColor(1, 3, 12));

  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 0, 7));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 0, 8));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 0, 9));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 0, 10));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 0, 11));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 1, 7));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 1, 8));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 1, 9));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 1, 10));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 1, 11));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 2, 7));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 2, 8));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 2, 9));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 2, 10));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 2, 11));
  // TODO(hamaji): ZENKESHI + OJAMA
  //EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 3, 2));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 3, 3));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 3, 4));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 3, 5));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 3, 6));
  //EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 4, 2));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 4, 3));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 4, 4));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 4, 5));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 4, 6));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 5, 1));
  //EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 5, 2));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 5, 3));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 5, 4));
  EXPECT_EQ(Capture::RC_OJAMA, cap->getRealColor(0, 5, 5));
}

TEST_F(CaptureTest, start_effect_nico) {
  auto_ptr<Capture> cap(createCapture("test/start_effect_nico1.gif"));
  EXPECT_EQ(Capture::RC_EMPTY, cap->getRealColor(1, 4, 11));
}

int main(int argc, char **argv) {
  FLAGS_commentator = false;
  SDL_Init(SDL_INIT_VIDEO);
  testing::InitGoogleTest(&argc, argv);
  int r = RUN_ALL_TESTS();
  SDL_Quit();
  return r;
}
