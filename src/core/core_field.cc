#include "core/core_field.h"

#include <array>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "core/column_puyo.h"
#include "core/column_puyo_list.h"
#include "core/constant.h"
#include "core/decision.h"
#include "core/field_bit_field.h"
#include "core/frame.h"
#include "core/kumipuyo.h"
#include "core/position.h"
#include "core/rensa_result.h"
#include "core/rensa_tracker.h"

using namespace std;

CoreField::CoreField(const std::string& url) :
    field_(url)
{
    heights_[0] = 0;
    for (int x = 1; x <= WIDTH; ++x) {
        heights_[x] = 13;
        for (int y = 1; y <= 13; ++y) {
            if (color(x, y) == PuyoColor::EMPTY) {
                heights_[x] = y - 1;
                break;
            }
        }
        for (int y = heights_[x] + 1; y <= 13; ++y)
            DCHECK(isEmpty(x, y));
    }
    heights_[MAP_WIDTH - 1] = 0;
}

CoreField::CoreField(const PlainField& f) :
    field_(f)
{
    heights_[0] = 0;
    for (int x = 1; x <= WIDTH; ++x) {
        heights_[x] = 13;
        for (int y = 1; y <= 13; ++y) {
            if (color(x, y) == PuyoColor::EMPTY) {
                heights_[x] = y - 1;
                break;
            }
        }
        for (int y = heights_[x] + 1; y <= 13; ++y)
            DCHECK(isEmpty(x, y));
    }
    heights_[MAP_WIDTH - 1] = 0;
}

bool CoreField::isZenkeshi() const
{
    for (int x = 1; x <= WIDTH; ++x) {
        if (height(x) > 0)
            return false;
    }

    return true;
}

int CoreField::countPuyos() const
{
    int count = 0;
    for (int x = 1; x <= WIDTH; ++x)
        count += height(x);

    return count;
}

int CoreField::countColorPuyos() const
{
    int cnt = 0;
    for (int x = 1; x <= WIDTH; ++x) {
        for (int y = 1; y <= height(x); ++y) {
            if (isNormalColor(color(x, y)))
                ++cnt;
        }
    }

    return cnt;
}

int CoreField::countConnectedPuyos(int x, int y) const
{
    FieldBitField checked;
    return countConnectedPuyos(x, y, &checked);
}

int CoreField::countConnectedPuyos(int x, int y, FieldBitField* checked) const
{
    Position positions[WIDTH * HEIGHT];

    Position* filledHead = fillSameColorPosition(x, y, color(x, y), positions, checked);
    return filledHead - positions;
}

int CoreField::countConnectedPuyosMax4(int x, int y) const
{
    bool leftUp = false, leftDown = false, rightUp = false, rightDown = false;
    int cnt = 1;
    PuyoColor c = color(x, y);

    if (color(x - 1, y) == c) {
        if (color(x - 2, y) == c) {
            if (color(x - 3, y) == c || (color(x - 2, y + 1) == c && y + 1 <= 12) || color(x - 2, y - 1) == c)
                return 4;
            ++cnt;
        }
        if (color(x - 1, y + 1) == c && y + 1 <= 12) {
            if (color(x - 2, y + 1) == c || (color(x - 1, y + 2) == c && y + 2 <= 12))
                return 4;
            ++cnt;
            leftUp = true;
        }
        if (color(x - 1, y - 1) == c) {
            if (color(x - 2, y - 1) == c || color(x - 1, y - 2) == c)
                return 4;
            ++cnt;
            leftDown = true;
        }
        ++cnt;
    }
    if (color(x + 1, y) == c) {
        if (color(x + 2, y) == c) {
            if (color(x + 3, y) == c || (color(x + 2, y + 1) == c && y + 1 <= 12) || color(x + 2, y - 1) == c)
                return 4;
            ++cnt;
        }
        if (color(x + 1, y + 1) == c && y + 1 <= 12) {
            if (color(x + 2, y + 1) == c || (color(x + 1, y + 2) == c && y + 2 <= 12))
                return 4;
            ++cnt;
            rightUp = true;
        }
        if (color(x + 1, y - 1) == c) {
            if (color(x + 2, y - 1) == c || color(x + 1, y - 2) == c)
                return 4;
            ++cnt;
            rightDown = true;
        }
        ++cnt;
    }
    if (color(x, y - 1) == c) {
        if (color(x, y - 2) == c) {
            if (color(x, y - 3) == c || color(x - 1, y - 2) == c || color(x + 1, y - 2) == c)
                return 4;
            ++cnt;
        }
        if (color(x - 1, y - 1) == c && !leftDown) {
            if (color(x - 2, y - 1) == c || color(x - 1, y - 2) == c)
                return 4;
            ++cnt;
        }
        if (color(x + 1, y - 1) == c && !rightDown) {
            if (color(x + 2, y - 1) == c || color(x + 1, y - 2) == c)
                return 4;
            ++cnt;
        }
        ++cnt;
    }
    if (color(x, y + 1) == c && y + 1 <= 12) {
        if (color(x, y + 2) == c && y + 2 <= 12) {
            if ((color(x, y + 3) == c && y + 3 <= 12) || color(x - 1, y + 2) == c || color(x + 1, y + 2) == c)
                return 4;
            ++cnt;
        }
        if (color(x - 1, y + 1) == c && !leftUp) {
            if (color(x - 2, y + 1) == c || (color(x - 1, y + 2) == c && y + 2 <= 12))
                return 4;
            ++cnt;
        }
        if (color(x + 1, y + 1) == c && !rightUp) {
            if (color(x + 2, y + 1) == c || (color(x + 1, y + 2) == c && y + 2 <= 12))
                return 4;
            ++cnt;
        }
        ++cnt;
    }

    return (cnt >= 5) ? 4 : cnt;
}

bool CoreField::isConnectedPuyo(int x, int y) const
{
    PuyoColor c = color(x, y);
    return color(x, y - 1) == c || color(x, y + 1) == c || color(x - 1, y) == c || color(x + 1, y) == c;
}

bool CoreField::dropKumipuyo(const Decision& decision, const Kumipuyo& kumiPuyo)
{
    int x1 = decision.axisX();
    int x2 = decision.childX();
    PuyoColor c1 = kumiPuyo.axis;
    PuyoColor c2 = kumiPuyo.child;

    if (decision.r == 2) {
        if (!dropPuyoOnWithMaxHeight(x2, c2, 14))
            return false;
        if (!dropPuyoOnWithMaxHeight(x1, c1, 13)) {
            removeTopPuyoFrom(x2);
            return false;
        }
        return true;
    }

    if (!dropPuyoOnWithMaxHeight(x1, c1, 13))
        return false;
    if (!dropPuyoOnWithMaxHeight(x2, c2, 14)) {
        removeTopPuyoFrom(x1);
        return false;
    }
    return true;
}

void CoreField::undoKumipuyo(const Decision& decision)
{
    removeTopPuyoFrom(decision.x);
    removeTopPuyoFrom(decision.childX());
}

int CoreField::framesToDropNext(const Decision& decision) const
{
    // TODO(mayah): This calculation should be more accurate. We need to compare this with
    // actual AC puyo2 and duel server algorithm. These must be much the same.

    // TODO(mayah): When "kabegoe" happens, we need more frames.
    const int KABEGOE_PENALTY = 6;

    // TODO(mayah): It looks drop animation is too short.

    int x1 = decision.axisX();
    int x2 = decision.childX();

    int dropFrames = FRAMES_TO_MOVE_HORIZONTALLY[abs(3 - x1)];

    if (decision.r == 0) {
        int dropHeight = HEIGHT - height(x1);
        if (dropHeight <= 0) {
            // TODO(mayah): We need to add penalty here. How much penalty is necessary?
            dropFrames += KABEGOE_PENALTY + FRAMES_GROUNDING;
        } else {
            dropFrames += FRAMES_TO_DROP_FAST[dropHeight] + FRAMES_GROUNDING;
        }
    } else if (decision.r == 2) {
        int dropHeight = HEIGHT - height(x1) - 1;
        // TODO: If puyo lines are high enough, rotation might take time. We should measure this later.
        // It looks we need 3 frames to waiting that each rotation has completed.
        if (dropHeight < 6)
            dropHeight = 6;

        dropFrames += FRAMES_TO_DROP_FAST[dropHeight] + FRAMES_GROUNDING;
    } else {
        if (height(x1) == height(x2)) {
            int dropHeight = HEIGHT - height(x1);
            if (dropHeight <= 0) {
                dropFrames += KABEGOE_PENALTY + FRAMES_GROUNDING;
            } else if (dropHeight < 3) {
                dropFrames += FRAMES_TO_DROP_FAST[3] + FRAMES_GROUNDING;
            } else {
                dropFrames += FRAMES_TO_DROP_FAST[dropHeight] + FRAMES_GROUNDING;
            }
        } else {
            int minHeight = min(height(x1), height(x2));
            int maxHeight = max(height(x1), height(x2));
            int diffHeight = maxHeight - minHeight;
            int dropHeight = HEIGHT - maxHeight;
            if (dropHeight <= 0) {
                dropFrames += KABEGOE_PENALTY;
            } else if (dropHeight < 3) {
                dropFrames += FRAMES_TO_DROP_FAST[3];
            } else {
                dropFrames += FRAMES_TO_DROP_FAST[dropHeight];
            }
            dropFrames += FRAMES_GROUNDING;
            dropFrames += FRAMES_TO_DROP[diffHeight];
            dropFrames += FRAMES_GROUNDING;
        }
    }

    CHECK(dropFrames >= 0);
    return dropFrames;
}

bool CoreField::isChigiriDecision(const Decision& decision) const
{
    DCHECK(decision.isValid()) << "decision " << decision.toString() << " should be valid.";

    if (decision.axisX() == decision.childX())
        return false;

    return height(decision.axisX()) != height(decision.childX());
}

int CoreField::fallOjama(int lines)
{
    if (lines == 0)
        return 0;

    int dropHeight = 0;
    for (int x = 1; x <= WIDTH; ++x) {
        dropHeight = std::max(dropHeight, 12 - height(x));
        for (int i = 0; i < lines; ++i) {
            (void)dropPuyoOnWithMaxHeight(x, PuyoColor::OJAMA, 13);
        }
    }

    // TODO(mayah): When more ojamas are dropped, grounding frames is necessary more.
    // See FieldRealtime::onStateOjamaDropping() also.
    return FRAMES_TO_DROP[dropHeight] + framesGroundingOjama(6 * lines);
}

bool CoreField::dropPuyoOnWithMaxHeight(int x, PuyoColor c, int maxHeight)
{
    DCHECK_NE(c, PuyoColor::EMPTY) << toDebugString();
    DCHECK_LE(maxHeight, 14);

    if (height(x) >= maxHeight)
        return false;

    DCHECK_EQ(color(x, height(x) + 1), PuyoColor::EMPTY);
    unsafeSet(x, ++heights_[x], c);
    return true;
}

bool CoreField::dropPuyoListWithMaxHeight(const ColumnPuyoList& cpl, int maxHeight)
{
    for (int x = 1; x <= 6; ++x) {
        int s = cpl.sizeOn(x);
        for (int i = 0; i < s; ++i) {
            PuyoColor pc = cpl.get(x, i);
            if (!dropPuyoOnWithMaxHeight(x, pc, maxHeight))
                return false;
        }
    }

    return true;
}

int CoreField::fillErasingPuyoPositions(const SimulationContext& context, Position* eraseQueue) const
{
    Position* eraseQueueHead = eraseQueue;

    {
        FieldBitField checked;
        for (int x = 1; x <= WIDTH; ++x) {
            int maxHeight = height(x);
            for (int y = context.minHeights[x]; y <= maxHeight; ++y) {
                DCHECK_NE(color(x, y), PuyoColor::EMPTY)
                    << x << ' ' << y << ' ' << toChar(color(x, y)) << '\n'
                    << toDebugString();

                if (checked.get(x, y))
                    continue;
                if (!isNormalColor(color(x, y)))
                    continue;

                PuyoColor c = color(x, y);
                Position* head = fillSameColorPosition(x, y, c, eraseQueueHead, &checked);

                int connectedPuyoNum = head - eraseQueueHead;
                if (connectedPuyoNum < PUYO_ERASE_NUM)
                    continue;

                eraseQueueHead = head;
            }
        }
    }

    int numErasedPuyos = eraseQueueHead - eraseQueue;
    if (numErasedPuyos == 0)
        return 0;

    Position* colorEraseQueueHead = eraseQueueHead;

    FieldBitField checked;
    for (Position* head = eraseQueue; head != colorEraseQueueHead; ++head) {
        int x = head->x;
        int y = head->y;

        // Check OJAMA puyos erased
        if (color(x + 1, y) == PuyoColor::OJAMA && !checked(x + 1, y)) {
            checked.set(x + 1, y);
            *eraseQueueHead++ = Position(x + 1, y);
        }

        if (color(x - 1, y) == PuyoColor::OJAMA && !checked(x - 1, y)) {
            checked.set(x - 1, y);
            *eraseQueueHead++ = Position(x + 1, y);
        }

        if (color(x, y + 1) == PuyoColor::OJAMA && y + 1 <= HEIGHT && !checked(x, y + 1)) {
            checked.set(x, y + 1);
            *eraseQueueHead++ = Position(x, y + 1);
        }

        if (color(x, y - 1) == PuyoColor::OJAMA && !checked(x, y - 1)) {
            checked.set(x, y - 1);
            *eraseQueueHead++ = Position(x, y - 1);
        }
    }

    return eraseQueueHead - eraseQueue;
}

vector<Position> CoreField::erasingPuyoPositions(const SimulationContext& context) const
{
    // All the positions of erased puyos will be stored here.
    Position eraseQueue[MAP_WIDTH * MAP_HEIGHT];
    int n = fillErasingPuyoPositions(context, eraseQueue);
    return vector<Position>(eraseQueue, eraseQueue + n);
}

bool CoreField::rensaWillOccurWhenLastDecisionIs(const Decision& decision) const
{
    Position p1 = Position(decision.x, height(decision.x));
    if (countConnectedPuyos(p1.x, p1.y) >= 4)
        return true;

    Position p2;
    switch (decision.r) {
    case 0:
    case 2:
        p2 = Position(decision.x, height(decision.x) - 1);
        break;
    case 1:
        p2 = Position(decision.x + 1, height(decision.x + 1));
        break;
    case 3:
        p2 = Position(decision.x - 1, height(decision.x - 1));
        break;
    default:
        DCHECK(false) << decision.toString();
        return false;
    }

    if (countConnectedPuyos(p2.x, p2.y) >= 4)
        return true;

    return false;
}

bool CoreField::rensaWillOccurWithContext(const SimulationContext& context) const
{
    for (int x = 1; x <= WIDTH; ++x) {
        int h = height(x);
        for (int y = context.minHeights[x]; y <= h; ++y) {
            if (!isNormalColor(color(x, y)))
                continue;

            if (countConnectedPuyosMax4(x, y) >= 4)
                return true;
        }
    }

    return false;
}

std::string CoreField::toDebugString() const
{
    std::ostringstream s;
    for (int y = MAP_HEIGHT - 1; y >= 0; y--) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            s << toChar(color(x, y)) << ' ';
        }
        s << std::endl;
    }
    s << ' ';
    for (int x = 1; x <= WIDTH; ++x)
        s << setw(2) << height(x);
    s << std::endl;
    return s.str();
}

// friend static
bool operator==(const CoreField& lhs, const CoreField& rhs)
{
    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        if (lhs.height(x) != rhs.height(x))
            return false;

        for (int y = 1; y <= lhs.height(x); ++y) {
            if (lhs.color(x, y) != rhs.color(x, y))
                return false;
        }
    }

    return true;
}

// friend static
bool operator!=(const CoreField& lhs, const CoreField& rhs)
{
    return !(lhs == rhs);
}

// instantiate CoreField::simulate().
template RensaResult CoreField::simulate(SimulationContext*, RensaNonTracker*);
template RensaResult CoreField::simulate(SimulationContext*, RensaChainTracker*);
template RensaResult CoreField::simulate(SimulationContext*, RensaCoefTracker*);
template RensaResult CoreField::simulate(SimulationContext*, RensaVanishingPositionTracker*);
