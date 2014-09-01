#include "core/ctrl.h"

#include <glog/logging.h>

#include "core/decision.h"
#include "core/kumipuyo.h"
#include "core/plain_field.h"

namespace {

// Remove redundant key stroke.
void removeRedundantKeySeq(const KumipuyoPos& pos, KeySetSeq* seq)
{
    switch (pos.r) {
    case 0:
        return;
    case 1:
        if (seq->size() >= 2 && (*seq)[0] == KeySet(Key::KEY_LEFT_TURN) && (*seq)[1] == KeySet(Key::KEY_RIGHT_TURN)) {
            // Remove 2 key strokes.
            seq->removeFront();
            seq->removeFront();
        }
        return;
    case 2:
        return;
    case 3:
        if (seq->size() >= 2 && (*seq)[0] == KeySet(Key::KEY_RIGHT_TURN) && (*seq)[1] == KeySet(Key::KEY_LEFT_TURN)) {
            seq->removeFront();
            seq->removeFront();
        }
        return;
    default:
        CHECK(false) << "Unknown r: " << pos.r;
        return;
    }
}

}

void Ctrl::moveKumipuyoByArrowKey(const PlainField& field, const KeySet& keySet, MovingKumipuyoState* mks, bool* downAccepted)
{
    DCHECK(mks);
    DCHECK(!mks->grounded) << "Grounded puyo cannot be moved.";

    // Only one key will be accepted.
    // When DOWN + RIGHT or DOWN + LEFT are simultaneously input, DOWN should be ignored.

    if (keySet.hasKey(Key::KEY_RIGHT)) {
        if (field.get(mks->pos.axisX() + 1, mks->pos.axisY()) == PuyoColor::EMPTY &&
            field.get(mks->pos.childX() + 1, mks->pos.childY()) == PuyoColor::EMPTY) {
            mks->pos.x++;
        }
        return;
    }

    if (keySet.hasKey(Key::KEY_LEFT)) {
        if (field.get(mks->pos.axisX() - 1, mks->pos.axisY()) == PuyoColor::EMPTY &&
            field.get(mks->pos.childX() - 1, mks->pos.childY()) == PuyoColor::EMPTY) {
            mks->pos.x--;
        }
        return;
    }

    if (keySet.hasKey(Key::KEY_DOWN)) {
        *downAccepted = true;
        if (mks->restFramesForFreefall > 0) {
            mks->restFramesForFreefall = 0;
            return;
        }

        mks->restFramesForFreefall = 0;
        if (field.get(mks->pos.axisX(), mks->pos.axisY() - 1) == PuyoColor::EMPTY &&
            field.get(mks->pos.childX(), mks->pos.childY() - 1) == PuyoColor::EMPTY) {
            mks->pos.y--;
            return;
        }

        // Grounded.
        mks->grounded = true;
        return;
    }
}

void Ctrl::moveKumipuyoByTurnKey(const PlainField& field, const KeySet& keySet, MovingKumipuyoState* mks)
{
    if (keySet.hasKey(Key::KEY_RIGHT_TURN)) {
        switch (mks->pos.r) {
        case 0:
            if (field.get(mks->pos.x + 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 1) % 4;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }
            if (field.get(mks->pos.x - 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 1) % 4;
                mks->pos.x--;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (mks->restFramesToAcceptQuickTurn > 0) {
                mks->pos.r = 2;
                mks->pos.y++;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            mks->restFramesToAcceptQuickTurn = FRAMES_QUICKTURN;
            return;
        case 1:
            if (field.get(mks->pos.x, mks->pos.y - 1) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 1) % 4;
                return;
            }

            mks->pos.r = (mks->pos.r + 1) % 4;
            mks->pos.y++;
            return;
        case 2:
            if (field.get(mks->pos.x - 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 1) % 4;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (field.get(mks->pos.x + 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 1) % 4;
                mks->pos.x++;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (mks->restFramesToAcceptQuickTurn > 0) {
                mks->pos.r = 0;
                mks->pos.y--;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            mks->restFramesToAcceptQuickTurn = FRAMES_QUICKTURN;
            return;
        case 3:
            mks->pos.r = (mks->pos.r + 1) % 4;
            return;
        default:
            CHECK(false) << mks->pos.r;
            return;
        }
    }

    if (keySet.hasKey(Key::KEY_LEFT_TURN)) {
        switch (mks->pos.r) {
        case 0:
            if (field.get(mks->pos.x - 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 3) % 4;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (field.get(mks->pos.x + 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 3) % 4;
                mks->pos.x++;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (mks->restFramesToAcceptQuickTurn > 0) {
                mks->pos.r = 2;
                mks->pos.y++;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            mks->restFramesToAcceptQuickTurn = FRAMES_QUICKTURN;
            return;
        case 1:
            mks->pos.r = (mks->pos.r + 3) % 4;
            return;
        case 2:
            if (field.get(mks->pos.x + 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 3) % 4;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (field.get(mks->pos.x - 1, mks->pos.y) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 3) % 4;
                mks->pos.x--;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            if (mks->restFramesToAcceptQuickTurn > 0) {
                mks->pos.r = 0;
                mks->pos.y--;
                mks->restFramesToAcceptQuickTurn = 0;
                return;
            }

            mks->restFramesToAcceptQuickTurn = FRAMES_QUICKTURN;
            return;
        case 3:
            if (field.get(mks->pos.x, mks->pos.y - 1) == PuyoColor::EMPTY) {
                mks->pos.r = (mks->pos.r + 3) % 4;
                return;
            }

            mks->pos.r = (mks->pos.r + 3) % 4;
            mks->pos.y++;
            return;
        default:
            CHECK(false) << mks->pos.r;
            return;
        }
    }
}

void Ctrl::moveKumipuyoByFreefall(const PlainField& field, MovingKumipuyoState* mks)
{
    DCHECK(!mks->grounded);

    if (mks->restFramesForFreefall > 1) {
        mks->restFramesForFreefall--;
        return;
    }

    mks->restFramesForFreefall = FRAMES_FREE_FALL;
    if (field.get(mks->pos.axisX(), mks->pos.axisY() - 1) == PuyoColor::EMPTY &&
        field.get(mks->pos.childX(), mks->pos.childY() - 1) == PuyoColor::EMPTY) {
        mks->pos.y--;
        return;
    }

    mks->grounded = true;
    return;
}

void Ctrl::moveKumipuyo(const PlainField& field, const KeySet& keySet, MovingKumipuyoState* mks, bool* downAccepted)
{
    // TODO(mayah): Which key is consumed first? turn? arrow?
    moveKumipuyoByArrowKey(field, keySet, mks, downAccepted);
    moveKumipuyoByTurnKey(field, keySet, mks);
    if (!keySet.hasKey(Key::KEY_DOWN))
        moveKumipuyoByFreefall(field, mks);
}

bool Ctrl::isReachable(const PlainField& field, const Decision& decision)
{
    if (isReachableFastpath(field, decision))
        return true;

    // slowpath
    return isReachableOnline(field, KumipuyoPos(decision.x, 1, decision.r), KumipuyoPos::InitialPos());
}

bool Ctrl::isReachableFastpath(const PlainField& field, const Decision& decision)
{
    DCHECK(decision.isValid()) << decision.toString();

    static const int checker[6][4] = {
        { 2, 1, 0 },
        { 2, 0 },
        { 0 },
        { 4, 0 },
        { 4, 5, 0 },
        { 4, 5, 6, 0 },
    };

    int checkerIdx = decision.x - 1;
    if (decision.r == 1 && 3 <= decision.x)
        checkerIdx += 1;
    else if (decision.r == 3 && decision.x <= 3)
        checkerIdx -= 1;

    // When decision is valid, this should hold.
    DCHECK(0 <= checkerIdx && checkerIdx < 6) << checkerIdx;

    for (int i = 0; checker[checkerIdx][i] != 0; ++i) {
        if (field.get(checker[checkerIdx][i], 12) != PuyoColor::EMPTY)
            return false;
    }

    return true;
}

bool Ctrl::isReachableOnline(const PlainField& field, const KumipuyoPos& goal, const KumipuyoPos& start)
{
    KeySetSeq keySetSeq;
    return getControlOnline(field, goal, start, &keySetSeq);
}

bool Ctrl::isQuickturn(const PlainField& field, const KumipuyoPos& k)
{
    // assume that k.r == 0 or 2
    return (field.get(k.x - 1, k.y) != PuyoColor::EMPTY && field.get(k.x + 1, k.y) != PuyoColor::EMPTY);
}

bool Ctrl::getControl(const PlainField& field, const Decision& decision, KeySetSeq* ret)
{
    ret->clear();

    if (!isReachable(field, decision)) {
        return false;
    }
    int x = decision.x;
    int r = decision.r;

    switch (r) {
    case 0:
        moveHorizontally(x - 3, ret);
        break;

    case 1:
        add(Key::KEY_RIGHT_TURN, ret);
        moveHorizontally(x - 3, ret);
        break;

    case 2:
        moveHorizontally(x - 3, ret);
        if (x < 3) {
            add(Key::KEY_RIGHT_TURN, ret);
            add(Key::KEY_RIGHT_TURN, ret);
        } else if (x > 3) {
            add(Key::KEY_LEFT_TURN, ret);
            add(Key::KEY_LEFT_TURN, ret);
        } else {
            if (field.get(4, 12) != PuyoColor::EMPTY) {
                if (field.get(2, 12) != PuyoColor::EMPTY) {
                    // fever's quick turn
                    add(Key::KEY_RIGHT_TURN, ret);
                    add(Key::KEY_RIGHT_TURN, ret);
                } else {
                    add(Key::KEY_LEFT_TURN, ret);
                    add(Key::KEY_LEFT_TURN, ret);
                }
            } else {
                add(Key::KEY_RIGHT_TURN, ret);
                add(Key::KEY_RIGHT_TURN, ret);
            }
        }
        break;

    case 3:
        add(Key::KEY_LEFT_TURN, ret);
        moveHorizontally(x - 3, ret);
        break;

    default:
        LOG(FATAL) << r;
    }

    add(Key::KEY_DOWN, ret);

    removeRedundantKeySeq(KumipuyoPos::InitialPos(), ret);

    return true;
}

// returns null if not reachable
bool Ctrl::getControlOnline(const PlainField& field, const KumipuyoPos& goal, const KumipuyoPos& start, KeySetSeq* ret)
{
    KumipuyoPos current = start;

    ret->clear();
    while (true) {
        if (goal.x == current.x && goal.r == current.r) {
            break;
        }

        // for simpicity, direct child-puyo upwards
        // TODO(yamaguchi): eliminate unnecessary moves
        if (current.r == 1) {
            add(Key::KEY_LEFT_TURN, ret);
            current.r = 0;
        } else if (current.r == 3) {
            add(Key::KEY_RIGHT_TURN, ret);
            current.r = 0;
        } else if (current.r == 2) {
            if (isQuickturn(field, current)) {
                // do quick turn
                add(Key::KEY_RIGHT_TURN, ret);
                add(Key::KEY_RIGHT_TURN, ret);
                current.y++;
            } else {
                if (field.get(current.x - 1, current.y) != PuyoColor::EMPTY) {
                    add(Key::KEY_LEFT_TURN, ret);
                    add(Key::KEY_LEFT_TURN, ret);
                } else {
                    add(Key::KEY_RIGHT_TURN, ret);
                    add(Key::KEY_RIGHT_TURN, ret);
                }
            }
            current.r = 0;
        }
        if (goal.x == current.x) {
            switch(goal.r) {
            case 0:
                break;
            case 1:
                if (field.get(current.x + 1, current.y) != PuyoColor::EMPTY) {
                    if (field.get(current.x + 1, current.y + 1) != PuyoColor::EMPTY ||
                        field.get(current.x, current.y - 1) == PuyoColor::EMPTY) {
                        return false;
                    }
                    // turn inversely to avoid kicking wall
                    add(Key::KEY_LEFT_TURN, ret);
                    add(Key::KEY_LEFT_TURN, ret);
                    add(Key::KEY_LEFT_TURN, ret);
                } else {
                    add(Key::KEY_RIGHT_TURN, ret);
                }
                break;
            case 3:
                if (field.get(current.x - 1, current.y) != PuyoColor::EMPTY) {
                    if (field.get(current.x - 1, current.y + 1) != PuyoColor::EMPTY ||
                        field.get(current.x, current.y - 1) == PuyoColor::EMPTY) {
                        return false;
                    }
                    add(Key::KEY_RIGHT_TURN, ret);
                    add(Key::KEY_RIGHT_TURN, ret);
                    add(Key::KEY_RIGHT_TURN, ret);
                } else {
                    add(Key::KEY_LEFT_TURN, ret);
                }
                break;
            case 2:
                if (field.get(current.x - 1, current.y) != PuyoColor::EMPTY) {
                    add(Key::KEY_RIGHT_TURN, ret);
                    add(Key::KEY_RIGHT_TURN, ret);
                } else {
                    add(Key::KEY_LEFT_TURN, ret);
                    add(Key::KEY_LEFT_TURN, ret);
                }
                break;
            }
            break;
        }

        // direction to move horizontally
        if (goal.x > current.x) {
            // move to right
            if (field.get(current.x + 1, current.y) == PuyoColor::EMPTY) {
                add(Key::KEY_RIGHT, ret);
                current.x++;
            } else {  // hits a wall
                // climb if possible
                /*
                  aBb
                  .A@
                  .@@.
                */
                // pivot puyo cannot go up anymore
                if (current.y >= 13) return false;
                // check "b"
                if (field.get(current.x + 1, current.y + 1) != PuyoColor::EMPTY) {
                    return false;
                }
                if (field.get(current.x, current.y - 1) != PuyoColor::EMPTY || isQuickturn(field, current)) {
                    // can climb by kicking the ground or quick turn. In either case,
                    // kumi-puyo is never moved because right side is blocked

                    add(Key::KEY_LEFT_TURN, ret);
                    add(Key::KEY_LEFT_TURN, ret);
                    current.y++;
                    if (!field.get(current.x - 1, current.y + 1)) {
                        add(Key::KEY_RIGHT_TURN, ret);
                        add(Key::KEY_RIGHT, ret);
                    } else {
                        // if "a" in the figure is filled, kicks wall. we can omit right key.
                        add(Key::KEY_RIGHT_TURN, ret);
                    }
                    add(Key::KEY_RIGHT_TURN, ret);
                    current.x++;
                } else {
                    return false;
                }
            }
        } else {
            // move to left
            if (!field.get(current.x - 1, current.y)) {
                add(Key::KEY_LEFT, ret);
                current.x--;
            } else {  // hits a wall
                // climb if possible
                /*
                  bBa
                  @A.
                  @@@.
                */
                // pivot puyo cannot go up anymore
                if (current.y >= 13) return false;
                // check "b"
                if (field.get(current.x - 1, current.y + 1) != PuyoColor::EMPTY) {
                    return false;
                }
                if (field.get(current.x, current.y - 1) != PuyoColor::EMPTY || isQuickturn(field, current)) {
                    // can climb by kicking the ground or quick turn. In either case,
                    // kumi-puyo is never moved because left side is blocked
                    add(Key::KEY_RIGHT_TURN, ret);
                    add(Key::KEY_RIGHT_TURN, ret);
                    current.y++;
                    if (!field.get(current.x + 1, current.y)) {
                        add(Key::KEY_LEFT_TURN, ret);
                        add(Key::KEY_LEFT, ret);
                    } else {
                        // if "a" in the figure is filled, kicks wall. we can omit left key.
                        add(Key::KEY_LEFT_TURN, ret);
                    }
                    add(Key::KEY_LEFT_TURN, ret);
                    current.x--;
                } else {
                    return false;
                }
            }
        }
    }

    add(Key::KEY_DOWN, ret);
    removeRedundantKeySeq(start, ret);
    // LOG(INFO) << buttonsDebugString();
    return true;
}

void Ctrl::add(Key b, KeySetSeq* ret)
{
    ret->add(KeySet(b));
}

void Ctrl::moveHorizontally(int x, KeySetSeq* ret)
{
    if (x < 0) {
        for (int i = 0; i < -x; i++) {
            add(Key::KEY_LEFT, ret);
        }
    } else if (x > 0) {
        for (int i = 0; i < x; i++) {
            add(Key::KEY_RIGHT, ret);
        }
    }
}
