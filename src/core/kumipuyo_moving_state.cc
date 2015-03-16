#include "core/kumipuyo_moving_state.h"

#include <glog/logging.h>

#include <ostream>
#include <tuple>

#include "core/constant.h"
#include "core/key.h"
#include "core/key_set.h"
#include "core/plain_field.h"
#include "core/puyo_color.h"

// TODO(mayah): Why these definitions are required for gtest?
const int KumipuyoMovingState::FRAMES_CONTINUOUS_TURN_PROHIBITED;
const int KumipuyoMovingState::FRAMES_CONTINUOUS_ARROW_PROHIBITED;

bool operator==(const KumipuyoMovingState& lhs, const KumipuyoMovingState& rhs)
{
    return std::tie(lhs.pos,
                    lhs.restFramesTurnProhibited,
                    lhs.restFramesArrowProhibited,
                    lhs.restFramesToAcceptQuickTurn,
                    lhs.restFramesForFreefall,
                    lhs.numGrounded,
                    lhs.grounding,
                    lhs.grounded) ==
        std::tie(rhs.pos,
                 rhs.restFramesTurnProhibited,
                 rhs.restFramesArrowProhibited,
                 rhs.restFramesToAcceptQuickTurn,
                 rhs.restFramesForFreefall,
                 rhs.numGrounded,
                 rhs.grounding,
                     rhs.grounded);
}

bool operator<(const KumipuyoMovingState& lhs, const KumipuyoMovingState& rhs)
{
    return std::tie(lhs.pos,
                    lhs.restFramesTurnProhibited,
                    lhs.restFramesArrowProhibited,
                    lhs.restFramesToAcceptQuickTurn,
                    lhs.restFramesForFreefall,
                    lhs.numGrounded,
                    lhs.grounding,
                    lhs.grounded) <
        std::tie(rhs.pos,
                 rhs.restFramesTurnProhibited,
                 rhs.restFramesArrowProhibited,
                 rhs.restFramesToAcceptQuickTurn,
                 rhs.restFramesForFreefall,
                 rhs.numGrounded,
                 rhs.grounding,
                 rhs.grounded);
}

void KumipuyoMovingState::moveKumipuyo(const PlainField& field, const KeySet& keySet, bool* downAccepted)
{
    if (restFramesToAcceptQuickTurn > 0)
        restFramesToAcceptQuickTurn--;

    // It looks turn key is consumed before arrow key.

    bool needsFreefallProcess = true;
    if (restFramesTurnProhibited > 0) {
        restFramesTurnProhibited--;
    } else {
        moveKumipuyoByTurnKey(field, keySet, &needsFreefallProcess);
    }

    if (!keySet.hasKey(Key::DOWN) && needsFreefallProcess)
        moveKumipuyoByFreefall(field);

    if (grounded)
        return;

    if (restFramesArrowProhibited > 0) {
        restFramesArrowProhibited--;
    } else {
        moveKumipuyoByArrowKey(field, keySet, downAccepted);
    }

    bool tmpGrounding = grounding;
    if (field.get(pos.axisX(), pos.axisY() - 1) != PuyoColor::EMPTY ||
        field.get(pos.childX(), pos.childY() - 1) != PuyoColor::EMPTY) {
        tmpGrounding |= restFramesForFreefall <= FRAMES_FREE_FALL / 2;
    } else {
        tmpGrounding = false;
    }

    if (tmpGrounding && !grounding) {
        restFramesForFreefall = FRAMES_FREE_FALL;
        grounding = true;
        numGrounded += 1;

        if (numGrounded >= 8) {
            grounded = true;
            return;
        }
    }
    if (!tmpGrounding && grounding) {
        restFramesForFreefall = FRAMES_FREE_FALL / 2;
        grounding = false;
    }
}

void KumipuyoMovingState::moveKumipuyoByArrowKey(const PlainField& field, const KeySet& keySet, bool* downAccepted)
{
    DCHECK(!grounded) << "Grounded puyo cannot be moved.";

    // Only one key will be accepted.
    // When DOWN + RIGHT or DOWN + LEFT are simultaneously input, DOWN should be ignored.

    if (keySet.hasKey(Key::RIGHT)) {
        restFramesArrowProhibited = FRAMES_CONTINUOUS_ARROW_PROHIBITED;
        if (field.get(pos.axisX() + 1, pos.axisY()) == PuyoColor::EMPTY &&
            field.get(pos.childX() + 1, pos.childY()) == PuyoColor::EMPTY) {
            pos.x++;
        }
        return;
    }

    if (keySet.hasKey(Key::LEFT)) {
        restFramesArrowProhibited = FRAMES_CONTINUOUS_ARROW_PROHIBITED;
        if (field.get(pos.axisX() - 1, pos.axisY()) == PuyoColor::EMPTY &&
            field.get(pos.childX() - 1, pos.childY()) == PuyoColor::EMPTY) {
            pos.x--;
        }
        return;
    }


    if (keySet.hasKey(Key::DOWN)) {
        // For DOWN key, we don't set restFramesArrowProhibited.
        if (downAccepted)
            *downAccepted = true;

        if (grounding) {
            restFramesForFreefall = 0;
            grounded = true;
            return;
        }

        if (restFramesForFreefall > 0) {
            restFramesForFreefall = 0;
            return;
        }

        restFramesForFreefall = 0;
        if (field.get(pos.axisX(), pos.axisY() - 1) == PuyoColor::EMPTY &&
            field.get(pos.childX(), pos.childY() - 1) == PuyoColor::EMPTY) {
            pos.y--;
            return;
        }

        // Grounded.
        grounded = true;
        return;
    }
}

void KumipuyoMovingState::moveKumipuyoByTurnKey(const PlainField& field, const KeySet& keySet, bool* needsFreefallProcess)
{
    DCHECK_EQ(0, restFramesTurnProhibited) << restFramesTurnProhibited;

    if (keySet.hasKey(Key::RIGHT_TURN)) {
        restFramesTurnProhibited = FRAMES_CONTINUOUS_TURN_PROHIBITED;
        switch (pos.r) {
        case 0:
            if (field.get(pos.x + 1, pos.y) == PuyoColor::EMPTY) {
                pos.r = (pos.r + 1) % 4;
                restFramesToAcceptQuickTurn = 0;
                return;
            }
            if (field.get(pos.x - 1, pos.y) == PuyoColor::EMPTY) {
                pos.r = (pos.r + 1) % 4;
                pos.x--;
                restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (restFramesToAcceptQuickTurn > 0) {
                pos.r = 2;
                pos.y++;
                restFramesToAcceptQuickTurn = 0;
                restFramesForFreefall = FRAMES_FREE_FALL / 2;
                *needsFreefallProcess = false;
                return;
            }

            restFramesToAcceptQuickTurn = FRAMES_QUICKTURN;
            return;
        case 1:
            if (field.get(pos.x, pos.y - 1) == PuyoColor::EMPTY) {
                pos.r = (pos.r + 1) % 4;
                return;
            }

            if (pos.y < 13) {
                pos.r = (pos.r + 1) % 4;
                pos.y++;
                restFramesForFreefall = FRAMES_FREE_FALL / 2;
                *needsFreefallProcess = false;
                return;
            }

            // The axis cannot be moved to 14th line.
            return;
        case 2:
            if (field.get(pos.x - 1, pos.y) == PuyoColor::EMPTY) {
                pos.r = (pos.r + 1) % 4;
                restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (field.get(pos.x + 1, pos.y) == PuyoColor::EMPTY) {
                pos.r = (pos.r + 1) % 4;
                pos.x++;
                restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (restFramesToAcceptQuickTurn > 0) {
                pos.r = 0;
                pos.y--;
                restFramesToAcceptQuickTurn = 0;
                return;
            }

            restFramesToAcceptQuickTurn = FRAMES_QUICKTURN;
            return;
        case 3:
            pos.r = (pos.r + 1) % 4;
            return;
        default:
            CHECK(false) << pos.r;
            return;
        }
    }

    if (keySet.hasKey(Key::LEFT_TURN)) {
        restFramesTurnProhibited = FRAMES_CONTINUOUS_TURN_PROHIBITED;
        switch (pos.r) {
        case 0:
            if (field.get(pos.x - 1, pos.y) == PuyoColor::EMPTY) {
                pos.r = (pos.r + 3) % 4;
                restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (field.get(pos.x + 1, pos.y) == PuyoColor::EMPTY) {
                pos.r = (pos.r + 3) % 4;
                pos.x++;
                restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (restFramesToAcceptQuickTurn > 0) {
                pos.r = 2;
                pos.y++;
                restFramesToAcceptQuickTurn = 0;
                restFramesForFreefall = FRAMES_FREE_FALL / 2;
                *needsFreefallProcess = false;
                return;
            }

            restFramesToAcceptQuickTurn = FRAMES_QUICKTURN;
            return;
        case 1:
            pos.r = (pos.r + 3) % 4;
            return;
        case 2:
            if (field.get(pos.x + 1, pos.y) == PuyoColor::EMPTY) {
                pos.r = (pos.r + 3) % 4;
                restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (field.get(pos.x - 1, pos.y) == PuyoColor::EMPTY) {
                pos.r = (pos.r + 3) % 4;
                pos.x--;
                restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (restFramesToAcceptQuickTurn > 0) {
                pos.r = 0;
                pos.y--;
                restFramesToAcceptQuickTurn = 0;
                return;
            }

            restFramesToAcceptQuickTurn = FRAMES_QUICKTURN;
            return;
        case 3:
            if (field.get(pos.x, pos.y - 1) == PuyoColor::EMPTY) {
                pos.r = (pos.r + 3) % 4;
                return;
            }

            if (pos.y < 13) {
                pos.r = (pos.r + 3) % 4;
                pos.y++;
                restFramesForFreefall = FRAMES_FREE_FALL / 2;
                *needsFreefallProcess = false;
                return;
            }

            // The axis cannot be moved to 14th line.
            return;
        default:
            CHECK(false) << pos.r;
            return;
        }
    }
}

void KumipuyoMovingState::moveKumipuyoByFreefall(const PlainField& field)
{
    DCHECK(!grounded);

    if (restFramesForFreefall > 1) {
        restFramesForFreefall--;
        return;
    }

    restFramesForFreefall = FRAMES_FREE_FALL;
    if (field.get(pos.axisX(), pos.axisY() - 1) == PuyoColor::EMPTY &&
        field.get(pos.childX(), pos.childY() - 1) == PuyoColor::EMPTY) {
        pos.y--;
        return;
    }

    grounded = true;
    return;
}
