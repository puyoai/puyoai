#include "field.h"

#include <cassert>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include <glog/logging.h>

#include "core/constant.h"
#include "core/decision.h"
#include "core/score.h"
#include "field_bit_field.h"
#include "plan.h"
#include "rensa_result.h"

using namespace std;

struct Position {
    Position() {}
    Position(int x, int y) : x(x), y(y) {}

    int x;
    int y;
};

class NonTrackingStrategy {
public:
    void colorPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) { }
    void ojamaPuyoIsVanished(int /*x*/, int /*y*/, int /*nthChain*/) { }
    void puyoIsDropped(int /*x*/, int /*fromY*/, int /*toY*/) { }
};

class RensaTrackingStrategy {
public:
    RensaTrackingStrategy(TrackResult& trackResult)
        : m_result(trackResult)
    {
        for (int x = 0; x < Field::MAP_WIDTH; ++x) {
            for (int y = 0; y < Field::MAP_HEIGHT; ++y) {
                m_originalY[x][y] = y;
                m_result.setErasedAt(x, y, 0);
            }
        }
    }

    void colorPuyoIsVanished(int x, int y, int nthChain) {
        m_result.setErasedAt(x, m_originalY[x][y], nthChain);
    }

    void ojamaPuyoIsVanished(int x, int y, int nthChain) {
        m_result.setErasedAt(x, m_originalY[x][y], nthChain);
    }

    void puyoIsDropped(int x, int fromY, int toY) {
        m_originalY[x][toY] = m_originalY[x][fromY];        
    }

private:
    int m_originalY[Field::MAP_WIDTH][Field::MAP_HEIGHT];
    TrackResult& m_result;
};

void Field::initialize()
{
    // Initialize field information.
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y)
            set(x, y, EMPTY);
    }

    for (int x = 0; x < MAP_WIDTH; ++x) {
        set(x, 0, WALL);
        set(x, MAP_HEIGHT - 1, WALL);
    }

    for (int y = 0; y < MAP_HEIGHT; ++y) {
        set(0, y, WALL);
        set(MAP_WIDTH - 1, y, WALL);
    }

    for (int x = 0; x < MAP_WIDTH; ++x)
        m_heights[x] = 0;
}

Field::Field()
{
    initialize();
}

Field::Field(const std::string& url)
{
    initialize();

    std::string prefix = "http://www.inosendo.com/puyo/rensim/??";
    int data_starts_at = url.find(prefix) == 0 ? prefix.length() : 0;

    int counter = 0;
    for (int i = url.length() - 1; i >= data_starts_at; --i) {
        int x = 6 - (counter % 6);
        int y = counter / 6 + 1;
        PuyoColor c = puyoColorOf(url[i]);
        set(x, y, c);
        if (c != EMPTY)
            m_heights[x] = y;
        counter++;
    }
}

Field::Field(const Field& f)
{
    for (int x = 0; x < MAP_WIDTH; x++) {
        m_heights[x] = f.m_heights[x];
        for (int y = 0; y < MAP_HEIGHT; y++) {
            set(x, y, f.color(x, y));
        }
    }
}

void Field::dropKumiPuyo(const Decision& decision, const KumiPuyo& kumiPuyo)
{
    int x1 = decision.x;
    int x2 = decision.x + (decision.r == 1) - (decision.r == 3);

    PuyoColor c1 = kumiPuyo.axis;
    PuyoColor c2 = kumiPuyo.child;
    if (decision.r == 2)
        swap(c1, c2);

    dropPuyoOn(x1, c1);
    dropPuyoOn(x2, c2);
}

void Field::dropKumiPuyoSafely(const Decision& decision, const KumiPuyo& kumiPuyo)
{
    int x1 = decision.x;
    int x2 = decision.x + (decision.r == 1) - (decision.r == 3);

    PuyoColor c1 = kumiPuyo.axis;
    PuyoColor c2 = kumiPuyo.child;
    if (decision.r == 2)
        swap(c1, c2);

    if (height(x1) < 14)
        dropPuyoOn(x1, c1);
    if (height(x2) < 14)
        dropPuyoOn(x2, c2);
}

int Field::framesToDropNext(const Decision& decision) const
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

void Field::dropPuyoOn(int x, PuyoColor c)
{
    DCHECK(c != EMPTY);
    DCHECK(height(x) < 14);
    DCHECK(color(x, height(x) + 1) == EMPTY) << " (x, height, color) = " << x << " : " << height(x) << " : " << color(x, height(x + 1));

    set(x, ++m_heights[x], c);
}

int Field::connectedPuyoNums(int x, int y) const
{
    FieldBitField checked;
    return connectedPuyoNums(x, y, checked);
}

int Field::connectedPuyoNums(int x, int y, FieldBitField& checked) const
{
    Position positions[WIDTH * HEIGHT];

    Position* filledHead = fillSameColorPosition(x, y, color(x, y), positions, checked);
    return filledHead - positions;    
}

pair<int, int> Field::connectedPuyoNumsWithAllowingOnePointJump(int x, int y) const
{
    FieldBitField checked;
    return connectedPuyoNumsWithAllowingOnePointJump(x, y, checked);
}

pair<int, int> Field::connectedPuyoNumsWithAllowingOnePointJump(int x, int y, FieldBitField& checked) const
{
    DCHECK(isColorPuyo(color(x, y)));
    Position positions[WIDTH * HEIGHT];
    int additional = 0;

    PuyoColor c = color(x, y);
    Position* end = fillSameColorPosition(x, y, c, positions, checked);
    Position* current = end;

    for (Position* p = positions; p != end; ++p) {
        if (1 <= p->x - 2 && color(p->x - 1, p->y) == EMPTY && color(p->x - 2, p->y) == c && !checked(p->x - 2, p->y)) {
            Position* newCurrent = fillSameColorPosition(p->x - 2, p->y, c, current, checked);
            if (newCurrent != current) {
                additional += 1;
                current = newCurrent;
            }
        }
        if (p->x + 2 <= Field::WIDTH && color(p->x + 1, p->y) == EMPTY && color(p->x + 2, p->y) == c && !checked(p->x + 2, p->y)) {
            Position* newCurrent = fillSameColorPosition(p->x + 2, p->y, c, current, checked);
            if (newCurrent != current) {
                additional += 1;
                current = newCurrent;
            }
        }
    }

    return make_pair(current - positions, additional);
}

bool Field::findBestBreathingSpace(int& breathingX, int& breathingY, int x, int y) const
{
    DCHECK(color(x, y) != EMPTY);

    FieldBitField checked;
    Position positions[WIDTH * HEIGHT];

    Position* filledHead = fillSameColorPosition(x, y, color(x, y), positions, checked);

    Position result;
    int resultHeight = 100;
    for (Position* p = positions; p != filledHead; ++p) {
        if (color(p->x, p->y + 1) == EMPTY) {
            result = Position(p->x, p->y + 1);
            resultHeight = 0;
            break;
        }

        if (color(p->x - 1, p->y) == EMPTY && p->y - height(p->x - 1) < resultHeight) {
            result = Position(p->x - 1, p->y);
            resultHeight = p->y - height(p->x - 1);
            if (resultHeight == 0)
                break;
        }

        if (color(p->x + 1, p->y) == EMPTY && p->y - height(p->x + 1) < resultHeight) {
            result = Position(p->x + 1, p->y);
            resultHeight = p->y - height(p->x + 1);
            if (resultHeight == 0)
                break;
        }
    }

    if (resultHeight >= 100)
        return false;

    breathingX = result.x;
    breathingY = result.y;
    return true;
}

int Field::countColorPuyos() const
{
    int cnt = 0;
    for (int x = 1; x <= WIDTH; ++x) {
        for (int y = 1; y <= height(x); ++y) {
            if (isColorPuyo(color(x, y)))
                ++cnt;
        }
    }

    return cnt;
}

int Field::countPuyos() const
{
    int count = 0;
    for (int x = 1; x <= WIDTH; ++x)
        count += height(x);

    return count;
}

inline Position* Field::checkCell(PuyoColor c, FieldBitField& checked, Position* writeHead, int x, int y) const
{
    if (checked(x, y))
        return writeHead;

    if (y <= HEIGHT && c == color(x, y)) {
        writeHead->x = x;
        writeHead->y = y;
        checked.set(x, y);

        return writeHead + 1;
    }

    return writeHead;
}

Position* Field::fillSameColorPosition(int x, int y, PuyoColor color, Position* positionQueueHead, FieldBitField& checked) const
{
    DCHECK(!checked(x, y));

    Position* writeHead = positionQueueHead;
    Position* readHead = positionQueueHead;

    *writeHead++ = Position(x, y);
    checked.set(x, y);
    
    while (readHead != writeHead) {
        Position p = *readHead++;
        
        writeHead = checkCell(color, checked, writeHead, p.x + 1, p.y);
        writeHead = checkCell(color, checked, writeHead, p.x - 1, p.y);
        writeHead = checkCell(color, checked, writeHead, p.x, p.y + 1);
        writeHead = checkCell(color, checked, writeHead, p.x, p.y - 1);
    }

    return writeHead;
}

template<typename Strategy>
bool Field::vanish(int nthChain, int* score, int minHeights[], Strategy& strategy)
{
    FieldBitField checked;
    Position eraseQueue[WIDTH * HEIGHT]; // All the positions of erased puyos will be stored here.
    Position* eraseQueueHead = eraseQueue;

    bool usedColors[PUYO_COLORS + 1] = { 0 };
    int numUsedColors = 0;
    int longBonusCoef = 0;

    for (int x = 1; x <= WIDTH; ++x) {
        int maxHeight = height(x);
        for (int y = minHeights[x]; y <= maxHeight; ++y) {
            DCHECK(color(x, y) != EMPTY)
                << x << ' ' << y << ' ' << color(x, y) << '\n'
                << getDebugOutput();

            if (checked(x, y) || color(x, y) == OJAMA)
                continue;

            PuyoColor c = color(x, y);
            Position* head = fillSameColorPosition(x, y, c, eraseQueueHead, checked);

            int connectedPuyoNum = head - eraseQueueHead;
            if (connectedPuyoNum < PUYO_ERASE_NUM)
                continue;

            eraseQueueHead = head;
            longBonusCoef += longBonus(connectedPuyoNum);
            if (!usedColors[c]) {
                ++numUsedColors;
                usedColors[c] = true;
            }
        }
    }

    int numErasedPuyos = eraseQueueHead - eraseQueue;
    if (numErasedPuyos == 0)
        return false;

    // --- Actually erase the Puyos to be vanished.
    eraseQueuedPuyos(nthChain, eraseQueue, eraseQueueHead, minHeights, strategy);

    int rensaBonusCoef = calculateRensaBonusCoef(chainBonus(nthChain), longBonusCoef, colorBonus(numUsedColors));
    *score += 10 * numErasedPuyos * rensaBonusCoef;
    return true;
}

template<typename Strategy>
void Field::eraseQueuedPuyos(int nthChain, Position* eraseQueue, Position* eraseQueueHead, int minHeights[], Strategy& strategy)
{
    for (int i = 1; i <= WIDTH; i++)
        minHeights[i] = 100;

    for (Position* head = eraseQueue; head != eraseQueueHead; ++head) {
        int x = head->x;
        int y = head->y;

        set(x, y, EMPTY);
        strategy.colorPuyoIsVanished(x, y, nthChain);
        minHeights[x] = std::min(minHeights[x], y);

        // Check OJAMA puyos erased
        if (color(x + 1, y) == OJAMA) {
            set(x + 1, y, EMPTY);
            strategy.ojamaPuyoIsVanished(x + 1, y, nthChain);
            minHeights[x + 1] = std::min(minHeights[x + 1], y);
        }

        if (color(x - 1, y) == OJAMA) {
            set(x - 1, y, EMPTY);
            strategy.ojamaPuyoIsVanished(x - 1, y, nthChain);
            minHeights[x - 1] = std::min(minHeights[x - 1], y);
        }

        // We don't need to update minHeights here.
        if (color(x, y + 1) == OJAMA && y + 1 <= HEIGHT) {
            set(x, y + 1, EMPTY);
            strategy.ojamaPuyoIsVanished(x, y + 1, nthChain);
        }

        if (color(x, y - 1) == OJAMA) {
            set(x, y - 1, EMPTY);
            strategy.ojamaPuyoIsVanished(x, y - 1, nthChain);
            minHeights[x] = std::min(minHeights[x], y - 1);
        }
    }
}

template<typename Strategy>
int Field::dropAfterVanish(int minHeights[], Strategy& strategy)
{
    int maxDrops = 0;
    for (int x = 1; x <= WIDTH; x++) {
        if (minHeights[x] >= MAP_HEIGHT)
            continue;
        int maxHeight = height(x);

        int writeAt = minHeights[x];
        m_heights[x] = writeAt - 1;

        DCHECK(color(x, writeAt) == EMPTY) << writeAt << ' ' << color(x, writeAt);
        for (int y = writeAt + 1; y <= maxHeight; ++y) {
            if (color(x, y) == EMPTY)
                continue;

            maxDrops = max(maxDrops, y - writeAt);
            set(x, writeAt, color(x, y));
            set(x, y, EMPTY);
            m_heights[x] = writeAt;
            strategy.puyoIsDropped(x, y, writeAt++);
        }
    }

    if (maxDrops == 0)
        return FRAMES_AFTER_NO_DROP;
    else
        return FRAMES_DROP_1_LINE * maxDrops + FRAMES_AFTER_DROP;
}

bool Field::isZenkeshi() const
{
    for (int x = 1; x <= WIDTH; ++x) {
        if (color(x, 1) != EMPTY)
            return false;
    }

    return true;
}

void Field::forceDrop()
{
    for (int x = 1; x <= Field::WIDTH; ++x) {
        int writeYAt = 1;
        for (int y = 1; y <= 13; ++y) {
            if (color(x, y) != EMPTY)
                set(x, writeYAt++, color(x, y));
        }
        for (int y = writeYAt; y <= 13; ++y)
            set(x, y, EMPTY);
        m_heights[x] = writeYAt - 1;
    }
}

void Field::simulate(BasicRensaInfo& rensaInfo, int additionalChain)
{
    rensaInfo.chains = 1 + additionalChain;
    rensaInfo.score = 0;
    rensaInfo.frames = 0;

    NonTrackingStrategy strategy;
    simulateWithStrategy(rensaInfo, strategy);
}

void Field::simulateAndTrack(BasicRensaInfo& rensaInfo, TrackResult& trackResult, int additionalChain)
{
    rensaInfo.chains = 1 + additionalChain;
    rensaInfo.score = 0;
    rensaInfo.frames = 0;

    RensaTrackingStrategy strategy(trackResult);
    simulateWithStrategy(rensaInfo, strategy);
}

template<typename Strategy>
inline void Field::simulateWithStrategy(BasicRensaInfo& rensaInfo, Strategy& strategy)
{
    int minHeights[MAP_WIDTH] = { 100, 1, 1, 1, 1, 1, 1, 100 };

    while (vanish(rensaInfo.chains, &rensaInfo.score, minHeights, strategy)) {
        rensaInfo.frames += dropAfterVanish(minHeights, strategy);
        rensaInfo.frames += FRAMES_AFTER_VANISH;
        rensaInfo.chains += 1;
    }

    rensaInfo.chains -= 1;
}

std::string Field::getDebugOutput() const
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

void Field::showDebugOutput() const
{
    cerr << getDebugOutput() << endl;
}

bool operator==(const Field& lhs, const Field& rhs)
{
    for (int x = 1; x <= Field::WIDTH; ++x) {
        if (lhs.height(x) != rhs.height(x))
            return false;

        for (int y = 1; y <= lhs.height(x); ++y) {
            if (lhs.color(x, y) != rhs.color(x, y))
                return false;
        }
    }

    return true;
}
