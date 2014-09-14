#ifndef CORE_CONSTANT_H_
#define CORE_CONSTANT_H_

// Actually, the internal FPS looks 60. However, we can take only 30 fps images
// when using video capture. We might be able to decompose 1 image to 2 frames
// if the video is interlace, though.
// For simplicity, we adopt 30 fps for now. However, we might want to have
// a room to adopt 60fps later.
const int FPS = 30;

const int FRAMES_PREPARING_NEXT = 6;
const int FRAMES_GROUNDING = 10;
const int FRAMES_VANISH_ANIMATION = 25;

const int FRAMES_NEXT2_DELAY = 8;
const int FRAMES_FREE_FALL = 8;
const int FRAMES_QUICKTURN = 10;

// TODO(mayah): We should consider that margin time has been expired.
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

// dropping after chigiri or dropping ojama puyo.
static const int FRAMES_TO_DROP[] = {
    0, 5, 8, 11, 12, 14, 16, 17, 18, 20, 21, 22, 23, 24, 25
};

// Pressing DOWN, or dropping after rensa.
static const int FRAMES_TO_DROP_FAST[] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14
};

//
static const int FRAMES_TO_MOVE_HORIZONTALLY[] = {
    0, 2, 3, 4, 5, 6
};

#endif  // CORE_CONSTANT_H_
