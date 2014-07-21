#ifndef CORE_DEPRECATED_CONSTAT_H_
#define CORE_DEPRECATED_CONSTAT_H_

// ----------------------------------------------------------------------
// Remove the following constants. They are not valid values now.

const int FRAMES_DROP_1_LINE = 1; // TODO(mayah): Remove this
const int FRAMES_AFTER_DROP = 12; // TODO(mayah): Remove this

// 9 frames of ground animation + 2 frames of connected puyos.
// 6 is FRAMES_AFTER_VANISH.
const int FRAMES_AFTER_CHIGIRI = 11 - 6; // TODO(mayah): Remove this

// After landing, there are 15 frames before next puyo comes.
// 6 is FRAMES_AFTER_VANISH.
const int FRAMES_AFTER_NO_CHIGIRI = 15 - 6; // TODO(mayah): Remove this

// Deprecated. Used by shinyak.
const int FRAMES_HORIZONTAL_MOVE = 1; // TODO(mayah): Remove this

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

#endif
