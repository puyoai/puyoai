#ifndef __CONSTANT_H__
#define __CONSTANT_H__

const int FRAMES_DROP_1_LINE = 3;
const int FRAMES_AFTER_DROP = 6;
const int FRAMES_CHIGIRI_1_LINE = 4;
const int FRAMES_AFTER_CHIGIRI = 6;
const int FRAMES_AFTER_NO_CHIGIRI = 1;
const int FRAMES_VANISH_ANIMATION = 60;
const int FRAMES_AFTER_VANISH = 6;
const int FRAMES_AFTER_NO_DROP = 1;
const int FRAMES_FREE_FALL = 60;
const int FRAMES_AFTER_USER_INTERACTION = 1;
const int FRAMES_QUICKTURN = 20;

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

#endif
