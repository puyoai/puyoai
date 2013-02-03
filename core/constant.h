#ifndef CORE_CONSTANT_H_
#define CORE_CONSTANT_H_

////////////////////////////////////////////////////////////////////////
// Confirmed.
const int FRAMES_AFTER_GROUND = 2;
const int FRAMES_AFTER_USER_INTERACTION = 1;
const int FRAMES_DROP_1_LINE = 1;
const int FRAMES_AFTER_DROP = 12;
const int FRAMES_YOKOKU_DELAY = 8;

// 9 frames of ground animation + 2 frames of connected puyos.
// 6 is FRAMES_AFTER_VANISH.
const int FRAMES_AFTER_CHIGIRI = 11 - 6;

// After landing, there are 15 frames before next puyo comes.
// 6 is FRAMES_AFTER_VANISH.
const int FRAMES_AFTER_NO_CHIGIRI = 15 - 6;

const int FRAMES_VANISH_ANIMATION = 25;

// Deprecated. Used by shinyak.
const int FRAMES_HORIZONTAL_MOVE = FRAMES_AFTER_USER_INTERACTION;

// Updated, but not accurate.
const int FRAMES_CHIGIRI_1_LINE_1 = 4;
const int FRAMES_CHIGIRI_1_LINE_2 = 3;
const int FRAMES_CHIGIRI_1_LINE_3 = 2;

////////////////////////////////////////////////////////////////////////
// To be confirmed.

// 2 frames are simply waiting. 4 frames are waiting for next puyos to appear.
const int FRAMES_AFTER_VANISH = 6;
const int FRAMES_AFTER_NO_DROP = 6;
// TODO: s/20/8/ once mugen mawashi issue has fixed.
const int FRAMES_FREE_FALL = 20;
const int FRAMES_QUICKTURN = 10;


////////////////////////////////////////////////////////////////////////
// Fixed.

const int FPS = 30;

const int SCORE_FOR_OJAMA = 70;
const int ZENKESHI_BONUS = SCORE_FOR_OJAMA * 6 * 5;

static const int CHAIN_BONUS[] = {
  0,   0,   8,  16,  32,  64,  96, 128, 160, 192,
  224, 256, 288, 320, 352, 384, 416, 448, 480, 512,
};
static const int COLOR_BONUS[] = {
  0, 0, 3, 6, 12, 24,
};
static const int LONG_BONUS[] = {
  0, 0, 0, 0, 0, 2, 3, 4, 5, 6, 7, 10,
};

#endif  // CORE_CONSTANT_H_
