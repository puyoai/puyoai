#ifndef CORE_FRAME_H_
#define CORE_FRAME_H_

// frame.h contains various constants and functions about frames.

#include <glog/logging.h>

// 1 seconds = 60 frames.
const int FPS = 60;

// The number of frames for preparing puyo. This frame is for animation that
// a puyo in NEXT is moving to a player field.
const int FRAMES_PREPARING_NEXT = 12;

// The number of frames for grounding animation of puyo.
const int FRAMES_GROUNDING = 20;

// The number of frames for vanishing animation of puyo.
const int FRAMES_VANISH_ANIMATION = 50;

// After this frames passed after the puyo is controllable, NEXT2 will be shown.
const int FRAMES_NEXT2_DELAY = 16;
const int FRAMES_FREE_FALL = 16;
const int FRAMES_QUICKTURN = 20;

const int FRAMES_CONTINUOUS_TURN_PROHIBITED = 3;
const int FRAMES_CONTINUOUS_ARROW_PROHIBITED = 3;

// dropping after chigiri or dropping ojama puyo.
static const int FRAMES_TO_DROP[] = {
    0, 10, 16, 22, 24, 28, 32, 34, 36, 40, 42, 44, 46, 48, 50, 52
};

// Pressing DOWN, or dropping after rensa.
static const int FRAMES_TO_DROP_FAST[] = {
    0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30
};

//
static const int FRAMES_TO_MOVE_HORIZONTALLY[] = {
    0, 4, 6, 8, 10, 12
};

// Returns the number of animation frames when ojama is grounding
inline int framesGroundingOjama(int numOjama)
{
    if (numOjama <= 0)
        return 0;

    // TODO(mayah): This is not accurate.

    if (numOjama <= 3)
        return 8;

    if (numOjama <= 18)
        return 24;

    return 32;
}

#endif
