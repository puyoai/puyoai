#ifndef CORE_CONSTANT_H_
#define CORE_CONSTANT_H_

// Actually, the internal FPS looks 60. However, we can take only 30 fps images
// when using video capture. We might be able to decompose 1 image to 2 frames
// if the video is interlace, though.
// For simplicity, we adopt 30 fps for now. However, we might want to have
// a room to adopt 60fps later.
const int FPS = 30;

const int FRAMES_PREPARING_NEXT = 6;
const int FRAMES_NEXT2_DELAY = 8;
const int FRAMES_GROUNDING = 10;
const int FRAMES_FREE_FALL = 8;
const int FRAMES_QUICKTURN = 10;
const int FRAMES_VANISH_ANIMATION = 25;

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

// ----------------------------------------------------------------------
// Remove the following constants. They are not valid values now.

const int FRAMES_AFTER_GROUND = 2; // TODO(mayah): Remove this
const int FRAMES_AFTER_USER_INTERACTION = 1; // TODO(mayah): Remove this
const int FRAMES_DROP_1_LINE = 1; // TODO(mayah): Remove this
const int FRAMES_AFTER_DROP = 12; // TODO(mayah): Remove this


// 9 frames of ground animation + 2 frames of connected puyos.
// 6 is FRAMES_AFTER_VANISH.
const int FRAMES_AFTER_CHIGIRI = 11 - 6; // TODO(mayah): Remove this

// After landing, there are 15 frames before next puyo comes.
// 6 is FRAMES_AFTER_VANISH.
const int FRAMES_AFTER_NO_CHIGIRI = 15 - 6; // TODO(mayah): Remove this


// Deprecated. Used by shinyak.
const int FRAMES_HORIZONTAL_MOVE = FRAMES_AFTER_USER_INTERACTION; // TODO(mayah): Remove this

// Updated, but not accurate.
const int FRAMES_CHIGIRI_1_LINE_1 = 5; // TODO(mayah): Remove this
const int FRAMES_CHIGIRI_1_LINE_2 = 3; // TODO(mayah): Remove this
const int FRAMES_CHIGIRI_1_LINE_3 = 2; // TODO(mayah): Remove this

////////////////////////////////////////////////////////////////////////
// To be confirmed.

const int FRAMES_USER_CAN_PLAY_AFTER_NEXT1AXIS_DISAPPEARED = 4; // TODO(mayah): Move this to wii/

// 2 frames are simply waiting. 4 frames are waiting for next puyos to appear.
const int FRAMES_AFTER_VANISH = 6; // TODO(mayah): Remove this
const int FRAMES_AFTER_NO_DROP = 6; // TODO(mayah): Remove this


////////////////////////////////////////////////////////////////////////
// Fixed.

#endif  // CORE_CONSTANT_H_
