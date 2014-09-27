#ifndef CORE_NEXT_PUYO_H_
#define CORE_NEXT_PUYO_H_

enum class NextPuyoPosition {
    CURRENT_AXIS,
    CURRENT_CHILD,
    NEXT1_AXIS,
    NEXT1_CHILD,
    NEXT2_AXIS,
    NEXT2_CHILD,
};

const int NUM_NEXT_PUYO_POSITION = 6;
constexpr NextPuyoPosition toNextPuyoPosition(int x) { return static_cast<NextPuyoPosition>(x); }

#endif
