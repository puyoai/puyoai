#ifndef CORE_FRAME_H_
#define CORE_FRAME_H_

#include <glog/logging.h>

#ifdef USE_FPS_60
const int FPS = 60;
const int FRAMES_PREPARING_NEXT = 12;
const int FRAMES_GROUNDING = 20;
const int FRAMES_VANISH_ANIMATION = 50;

const int FRAMES_NEXT2_DELAY = 16;
const int FRAMES_FREE_FALL = 16;
const int FRAMES_QUICKTURN = 20;

const int FRAMES_CONTINUOUS_TURN_PROHIBITED = 3;
const int FRAMES_CONTINUOUS_ARROW_PROHIBITED = 3;

// dropping after chigiri or dropping ojama puyo.
static const int FRAMES_TO_DROP[] = {
    0, 10, 16, 22, 24, 28, 32, 34, 36, 40, 42, 44, 46, 48, 50
};

// Pressing DOWN, or dropping after rensa.
static const int FRAMES_TO_DROP_FAST[] = {
    0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28
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

#else

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

const int FRAMES_CONTINUOUS_TURN_PROHIBITED = 1;
const int FRAMES_CONTINUOUS_ARROW_PROHIBITED = 1;

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

// Returns the number of animation frames when ojama is grounding
inline int framesGroundingOjama(int numOjama)
{
    if (numOjama <= 0)
        return 0;

    // TODO(mayah): This is not accurate.

    if (numOjama <= 3)
        return 4;

    if (numOjama <= 18)
        return 12;

    return 16;
}
#endif

#endif
