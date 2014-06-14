#include "core/field/core_field.h"

#include <iomanip>
#include <sstream>

#include "core/constant.h"
#include "core/decision.h"
#include "core/field/field_bit_field.h"
#include "core/field/rensa_result.h"
#include "core/kumipuyo.h"
#include "core/position.h"
#include "core/score.h"

using namespace std;

class RensaNonTracker {
public:
    void colorPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) { }
    void ojamaPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) { }
    void puyoIsDropped(int /*x*/, int /*fromY*/, int /*toY*/) { }
};

class RensaTracker {
public:
    RensaTracker(RensaTrackResult* trackResult) :
        m_result(trackResult)
    {
        DCHECK(trackResult);

        for (int x = 0; x < CoreField::MAP_WIDTH; ++x) {
            for (int y = 0; y < CoreField::MAP_HEIGHT; ++y) {
                m_originalY[x][y] = y;
                m_result->setErasedAt(x, y, 0);
            }
        }
    }

    void colorPuyoIsVanished(int x, int y, int nthChain) {
        m_result->setErasedAt(x, m_originalY[x][y], nthChain);
    }

    void ojamaPuyoIsVanished(int x, int y, int nthChain) {
        m_result->setErasedAt(x, m_originalY[x][y], nthChain);
    }

    void puyoIsDropped(int x, int fromY, int toY) {
        m_originalY[x][toY] = m_originalY[x][fromY];
    }

private:
    int m_originalY[CoreField::MAP_WIDTH][CoreField::MAP_HEIGHT];
    RensaTrackResult* m_result;
};

CoreField::CoreField()
{
    for (int x = 0; x < MAP_WIDTH; ++x)
        heights_[x] = 0;
}

CoreField::CoreField(const std::string& url) :
    PlainField(url)
{
    heights_[0] = 0;
    heights_[MAP_WIDTH - 1] = 0;
    for (int x = 1; x <= WIDTH; ++x)
        recalcHeightOn(x);
}

CoreField::CoreField(const CoreField& f) :
    PlainField(f)
{
    for (int x = 0; x < MAP_WIDTH; ++x)
        heights_[x] = f.heights_[x];
}

CoreField::CoreField(const PlainField& f) :
    PlainField(f)
{
    heights_[0] = 0;
    heights_[MAP_WIDTH - 1] = 0;
    for (int x = 1; x <= WIDTH; ++x)
        recalcHeightOn(x);
}

void CoreField::clear()
{
    *this = CoreField();
}

bool CoreField::isZenkeshi() const
{
    for (int x = 1; x <= WIDTH; ++x) {
        if (color(x, 1) != PuyoColor::EMPTY)
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

int CoreField::connectedPuyoNums(int x, int y) const
{
    FieldBitField checked;
    return connectedPuyoNums(x, y, &checked);
}

int CoreField::connectedPuyoNums(int x, int y, FieldBitField* checked) const
{
    Position positions[WIDTH * HEIGHT];

    Position* filledHead = fillSameColorPosition(x, y, color(x, y), positions, checked);
    return filledHead - positions;
}

void CoreField::forceDrop()
{
    for (int x = 1; x <= CoreField::WIDTH; ++x) {
        int writeYAt = 1;
        for (int y = 1; y <= 13; ++y) {
            if (color(x, y) != PuyoColor::EMPTY)
                unsafeSet(x, writeYAt++, color(x, y));
        }
        for (int y = writeYAt; y <= 13; ++y)
            unsafeSet(x, y, PuyoColor::EMPTY);
        heights_[x] = writeYAt - 1;
    }
}

bool CoreField::dropKumipuyo(const Decision& decision, const Kumipuyo& kumiPuyo)
{
    int x1 = decision.x;
    int x2 = decision.x + (decision.r == 1) - (decision.r == 3);

    PuyoColor c1 = kumiPuyo.axis;
    PuyoColor c2 = kumiPuyo.child;

    if (decision.r == 2) {
        if (!dropPuyoOn(x2, c2, false))
            return false;
        if (!dropPuyoOn(x1, c1, true)) {
            removeTopPuyoFrom(x2);
            return false;
        }
        return true;
    }

    if (!dropPuyoOn(x1, c1, true))
        return false;
    if (!dropPuyoOn(x2, c2, false)) {
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
    // TODO: This calculation should be more accurate.
    int x1 = decision.x;
    int x2 = decision.x + (decision.r == 1) - (decision.r == 3);

    int dropFrames = abs(3 - x1) * FRAMES_HORIZONTAL_MOVE;

    if (decision.r == 0)
        dropFrames += (HEIGHT - height(x1)) * FRAMES_DROP_1_LINE + FRAMES_AFTER_NO_CHIGIRI;
    else if (decision.r == 2) {
        // TODO: If puyo lines are high enough, rotation might take time. We should measure this later.
        if (height(x1) > 6)
            dropFrames += (HEIGHT - height(x1) + 1) * FRAMES_DROP_1_LINE + FRAMES_AFTER_NO_CHIGIRI;
        else
            dropFrames += (HEIGHT - height(x1) - 1) * FRAMES_DROP_1_LINE + FRAMES_AFTER_NO_CHIGIRI;
    } else {
        if (height(x1) == height(x2))
            dropFrames += (HEIGHT - height(x1)) * FRAMES_DROP_1_LINE + FRAMES_AFTER_NO_CHIGIRI;
        else {
            int minHeight = min(height(x1), height(x2));
            int maxHeight = max(height(x1), height(x2));
            int diff = maxHeight - minHeight;
            dropFrames += (HEIGHT - maxHeight) * FRAMES_DROP_1_LINE;
            dropFrames += FRAMES_AFTER_CHIGIRI;
            if (diff == 1)
                dropFrames += FRAMES_CHIGIRI_1_LINE_1;
            else if (diff == 2)
                dropFrames += FRAMES_CHIGIRI_1_LINE_1 + FRAMES_CHIGIRI_1_LINE_2;
            else
                dropFrames += FRAMES_CHIGIRI_1_LINE_1 + FRAMES_CHIGIRI_1_LINE_2 + (diff - 2) * FRAMES_CHIGIRI_1_LINE_3;
        }
    }

    if (dropFrames < 0)
        dropFrames = 0;

    return dropFrames;
}

bool CoreField::dropPuyoOn(int x, PuyoColor c, bool isAxis)
{
    DCHECK(c != PuyoColor::EMPTY);
    if (height(x) < 13) {
        DCHECK(color(x, height(x) + 1) == PuyoColor::EMPTY);
        unsafeSet(x, ++heights_[x], c);
        return true;
    }

    if (height(x) > 14)
        return false;
    if (isAxis)
        return false;

    unsafeSet(x, ++heights_[x], c);
    return true;
}

inline static
Position* checkCell(const CoreField& field, PuyoColor c, FieldBitField* checked, Position* writeHead, int x, int y)
{
    if (c != field.color(x, y))
        return writeHead;

    if (checked->get(x, y))
        return writeHead;

    if (y <= CoreField::HEIGHT) {
        *writeHead = Position(x, y);
        checked->set(x, y);

        return writeHead + 1;
    }

    return writeHead;
}

Position* CoreField::fillSameColorPosition(int x, int y, PuyoColor color, Position* positionQueueHead, FieldBitField* checked) const
{
    DCHECK(!checked->get(x, y));

    Position* writeHead = positionQueueHead;
    Position* readHead = positionQueueHead;

    *writeHead++ = Position(x, y);
    checked->set(x, y);

    while (readHead != writeHead) {
        Position p = *readHead++;

        writeHead = checkCell(*this, color, checked, writeHead, p.x + 1, p.y);
        writeHead = checkCell(*this, color, checked, writeHead, p.x - 1, p.y);
        writeHead = checkCell(*this, color, checked, writeHead, p.x, p.y + 1);
        writeHead = checkCell(*this, color, checked, writeHead, p.x, p.y - 1);
    }

    return writeHead;
}

int CoreField::vanishOnly(int currentNthChain)
{
    int minHeights[MAP_WIDTH] = { 100, 1, 1, 1, 1, 1, 1, 100 };
    RensaNonTracker nonTracker;
    return vanish(currentNthChain, minHeights, &nonTracker);
}

template<typename Tracker>
int CoreField::vanish(int nthChain, int minHeights[], Tracker* tracker)
{
    DCHECK(tracker);

    FieldBitField checked;
    Position eraseQueue[WIDTH * HEIGHT]; // All the positions of erased puyos will be stored here.
    Position* eraseQueueHead = eraseQueue;

    bool usedColors[PUYO_COLORS + 1] = { 0 };
    int numUsedColors = 0;
    int longBonusCoef = 0;

    for (int x = 1; x <= WIDTH; ++x) {
        int maxHeight = height(x);
        for (int y = minHeights[x]; y <= maxHeight; ++y) {
            DCHECK(color(x, y) != PuyoColor::EMPTY)
                << x << ' ' << y << ' ' << charOfPuyoColor(color(x, y)) << '\n'
                << debugOutput();

            if (checked.get(x, y) || color(x, y) == PuyoColor::OJAMA)
                continue;

            PuyoColor c = color(x, y);
            Position* head = fillSameColorPosition(x, y, c, eraseQueueHead, &checked);

            int connectedPuyoNum = head - eraseQueueHead;
            if (connectedPuyoNum < PUYO_ERASE_NUM)
                continue;

            eraseQueueHead = head;
            longBonusCoef += longBonus(connectedPuyoNum);
            if (!usedColors[static_cast<int>(c)]) {
                ++numUsedColors;
                usedColors[static_cast<int>(c)] = true;
            }
        }
    }

    int numErasedPuyos = eraseQueueHead - eraseQueue;
    if (numErasedPuyos == 0)
        return 0;

    // --- Actually erase the Puyos to be vanished.
    eraseQueuedPuyos(nthChain, eraseQueue, eraseQueueHead, minHeights, tracker);

    int rensaBonusCoef = calculateRensaBonusCoef(chainBonus(nthChain), longBonusCoef, colorBonus(numUsedColors));
    return 10 * numErasedPuyos * rensaBonusCoef;
}

template<typename Tracker>
void CoreField::eraseQueuedPuyos(int nthChain, Position* eraseQueue, Position* eraseQueueHead, int minHeights[], Tracker* tracker)
{
    DCHECK(tracker);

    for (int i = 1; i <= WIDTH; i++)
        minHeights[i] = 100;

    for (Position* head = eraseQueue; head != eraseQueueHead; ++head) {
        int x = head->x;
        int y = head->y;

        unsafeSet(x, y, PuyoColor::EMPTY);
        tracker->colorPuyoIsVanished(x, y, nthChain);
        minHeights[x] = std::min(minHeights[x], y);

        // Check OJAMA puyos erased
        if (color(x + 1, y) == PuyoColor::OJAMA) {
            unsafeSet(x + 1, y, PuyoColor::EMPTY);
            tracker->ojamaPuyoIsVanished(x + 1, y, nthChain);
            minHeights[x + 1] = std::min(minHeights[x + 1], y);
        }

        if (color(x - 1, y) == PuyoColor::OJAMA) {
            unsafeSet(x - 1, y, PuyoColor::EMPTY);
            tracker->ojamaPuyoIsVanished(x - 1, y, nthChain);
            minHeights[x - 1] = std::min(minHeights[x - 1], y);
        }

        // We don't need to update minHeights here.
        if (color(x, y + 1) == PuyoColor::OJAMA && y + 1 <= HEIGHT) {
            unsafeSet(x, y + 1, PuyoColor::EMPTY);
            tracker->ojamaPuyoIsVanished(x, y + 1, nthChain);
        }

        if (color(x, y - 1) == PuyoColor::OJAMA) {
            unsafeSet(x, y - 1, PuyoColor::EMPTY);
            tracker->ojamaPuyoIsVanished(x, y - 1, nthChain);
            minHeights[x] = std::min(minHeights[x], y - 1);
        }
    }
}

template<typename Tracker>
int CoreField::dropAfterVanish(int minHeights[], Tracker* tracker)
{
    DCHECK(tracker);

    int maxDrops = 0;
    for (int x = 1; x <= WIDTH; x++) {
        if (minHeights[x] >= MAP_HEIGHT)
            continue;
        int maxHeight = height(x);

        int writeAt = minHeights[x];
        heights_[x] = writeAt - 1;

        DCHECK(color(x, writeAt) == PuyoColor::EMPTY) << writeAt << ' ' << charOfPuyoColor(color(x, writeAt));
        for (int y = writeAt + 1; y <= maxHeight; ++y) {
            if (color(x, y) == PuyoColor::EMPTY)
                continue;

            maxDrops = max(maxDrops, y - writeAt);
            unsafeSet(x, writeAt, color(x, y));
            unsafeSet(x, y, PuyoColor::EMPTY);
            heights_[x] = writeAt;
            tracker->puyoIsDropped(x, y, writeAt++);
        }
    }

    if (maxDrops == 0)
        return FRAMES_AFTER_NO_DROP;
    else
        return FRAMES_DROP_1_LINE * maxDrops + FRAMES_AFTER_DROP;
}

bool CoreField::rensaWillOccurWhenLastDecisionIs(const Decision& decision) const
{
    Position p1 = Position(decision.x, height(decision.x));
    if (connectedPuyoNums(p1.x, p1.y) >= 4)
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

    if (connectedPuyoNums(p2.x, p2.y) >= 4)
        return true;

    return false;
}

RensaResult CoreField::simulate(int initialChain)
{
    RensaNonTracker tracker;
    return simulateWithTracker(initialChain, &tracker);
}

RensaResult CoreField::simulateAndTrack(RensaTrackResult* trackResult, int initialChain)
{
    DCHECK(trackResult);
    RensaTracker tracker(trackResult);
    return simulateWithTracker(initialChain, &tracker);
}

template<typename Tracker>
inline RensaResult CoreField::simulateWithTracker(int initialChain, Tracker* tracker)
{
    int minHeights[MAP_WIDTH] = { 100, 1, 1, 1, 1, 1, 1, 100 };
    int chains = initialChain, score = 0, frames = 0;

    int nthChainScore;
    while ((nthChainScore = vanish(chains, minHeights, tracker)) > 0) {
        chains += 1;
        score += nthChainScore;
        frames += dropAfterVanish(minHeights, tracker) + FRAMES_AFTER_VANISH;
    }

    return RensaResult(chains - 1, score, frames);
}

std::string CoreField::toString() const
{
    std::ostringstream s;
    for (int y = 1; y <= 13; ++y) {
        for (int x = 1; x <= WIDTH; ++x) {
            s << charOfPuyoColor(color(x, y));
        }
    }

    return s.str();
}

std::string CoreField::debugOutput() const
{
    std::ostringstream s;
    for (int y = MAP_HEIGHT - 1; y >= 0; y--) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            s << charOfPuyoColor(color(x, y)) << ' ';
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
