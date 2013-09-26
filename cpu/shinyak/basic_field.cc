#include "basic_field.h"

#include <iomanip>
#include <sstream>

#include "core/constant.h"
#include "core/decision.h"
#include "core/position.h"
#include "core/score.h"
#include "field_bit_field.h"
#include "rensa_result.h"

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

        for (int x = 0; x < Field::MAP_WIDTH; ++x) {
            for (int y = 0; y < Field::MAP_HEIGHT; ++y) {
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
    int m_originalY[Field::MAP_WIDTH][Field::MAP_HEIGHT];
    RensaTrackResult* m_result;
};

BasicField::BasicField()
{
    initialize();
}

BasicField::BasicField(const std::string& url)
{
    initialize();

    std::string prefix = "http://www.inosendo.com/puyo/rensim/??";
    int data_starts_at = url.find(prefix) == 0 ? prefix.length() : 0;

    int counter = 0;
    for (int i = url.length() - 1; i >= data_starts_at; --i) {
        int x = 6 - (counter % 6);
        int y = counter / 6 + 1;
        PuyoColor c = puyoColorOf(url[i]);
        setPuyo(x, y, c);
        if (c != EMPTY)
            m_heights[x] = y;
        counter++;
    }
}

BasicField::BasicField(const BasicField& f)
{
    for (int x = 0; x < MAP_WIDTH; x++) {
        m_heights[x] = f.m_heights[x];
        for (int y = 0; y < MAP_HEIGHT; y++) {
            setPuyo(x, y, f.color(x, y));
        }
    }
}

// TODO(mayah): It might be faster to copy from empty field
void BasicField::initialize()
{
    // Initialize field information.
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y)
            setPuyo(x, y, EMPTY);
    }

    for (int x = 0; x < MAP_WIDTH; ++x) {
        setPuyo(x, 0, WALL);
        setPuyo(x, MAP_HEIGHT - 1, WALL);
    }

    for (int y = 0; y < MAP_HEIGHT; ++y) {
        setPuyo(0, y, WALL);
        setPuyo(MAP_WIDTH - 1, y, WALL);
    }

    for (int x = 0; x < MAP_WIDTH; ++x)
        m_heights[x] = 0;
}

void BasicField::forceDrop()
{
    for (int x = 1; x <= Field::WIDTH; ++x) {
        int writeYAt = 1;
        for (int y = 1; y <= 13; ++y) {
            if (color(x, y) != EMPTY)
                setPuyo(x, writeYAt++, color(x, y));
        }
        for (int y = writeYAt; y <= 13; ++y)
            setPuyo(x, y, EMPTY);
        m_heights[x] = writeYAt - 1;
    }
}

void BasicField::dropKumiPuyo(const Decision& decision, const KumiPuyo& kumiPuyo)
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

int BasicField::framesToDropNext(const Decision& decision) const
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

void BasicField::dropPuyoOn(int x, PuyoColor c)
{
    DCHECK(c != EMPTY);
    DCHECK(height(x) < 14);
    DCHECK(color(x, height(x) + 1) == EMPTY) << " (x, height, color) = " << x << " : " << height(x) << " : " << color(x, height(x + 1));

    setPuyo(x, ++m_heights[x], c);
}

inline Position* BasicField::checkCell(PuyoColor c, FieldBitField& checked, Position* writeHead, int x, int y) const
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

Position* BasicField::fillSameColorPosition(int x, int y, PuyoColor color, Position* positionQueueHead, FieldBitField& checked) const
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

template<typename Tracker>
bool BasicField::vanish(int nthChain, int* score, int minHeights[], Tracker& tracker)
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
                << debugOutput();

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
    eraseQueuedPuyos(nthChain, eraseQueue, eraseQueueHead, minHeights, tracker);

    int rensaBonusCoef = calculateRensaBonusCoef(chainBonus(nthChain), longBonusCoef, colorBonus(numUsedColors));
    *score += 10 * numErasedPuyos * rensaBonusCoef;
    return true;
}

template<typename Tracker>
void BasicField::eraseQueuedPuyos(int nthChain, Position* eraseQueue, Position* eraseQueueHead, int minHeights[], Tracker& tracker)
{
    for (int i = 1; i <= WIDTH; i++)
        minHeights[i] = 100;

    for (Position* head = eraseQueue; head != eraseQueueHead; ++head) {
        int x = head->x;
        int y = head->y;

        setPuyo(x, y, EMPTY);
        tracker.colorPuyoIsVanished(x, y, nthChain);
        minHeights[x] = std::min(minHeights[x], y);

        // Check OJAMA puyos erased
        if (color(x + 1, y) == OJAMA) {
            setPuyo(x + 1, y, EMPTY);
            tracker.ojamaPuyoIsVanished(x + 1, y, nthChain);
            minHeights[x + 1] = std::min(minHeights[x + 1], y);
        }

        if (color(x - 1, y) == OJAMA) {
            setPuyo(x - 1, y, EMPTY);
            tracker.ojamaPuyoIsVanished(x - 1, y, nthChain);
            minHeights[x - 1] = std::min(minHeights[x - 1], y);
        }

        // We don't need to update minHeights here.
        if (color(x, y + 1) == OJAMA && y + 1 <= HEIGHT) {
            setPuyo(x, y + 1, EMPTY);
            tracker.ojamaPuyoIsVanished(x, y + 1, nthChain);
        }

        if (color(x, y - 1) == OJAMA) {
            setPuyo(x, y - 1, EMPTY);
            tracker.ojamaPuyoIsVanished(x, y - 1, nthChain);
            minHeights[x] = std::min(minHeights[x], y - 1);
        }
    }
}

template<typename Tracker>
int BasicField::dropAfterVanish(int minHeights[], Tracker& tracker)
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
            setPuyo(x, writeAt, color(x, y));
            setPuyo(x, y, EMPTY);
            m_heights[x] = writeAt;
            tracker.puyoIsDropped(x, y, writeAt++);
        }
    }

    if (maxDrops == 0)
        return FRAMES_AFTER_NO_DROP;
    else
        return FRAMES_DROP_1_LINE * maxDrops + FRAMES_AFTER_DROP;
}

void BasicField::simulate(BasicRensaResult& rensaResult, int additionalChain)
{
    rensaResult.chains = 1 + additionalChain;
    rensaResult.score = 0;
    rensaResult.frames = 0;

    RensaNonTracker tracker;
    simulateWithTracker(rensaResult, tracker);
}

void BasicField::simulateAndTrack(BasicRensaResult& rensaResult, RensaTrackResult& trackResult, int additionalChain)
{
    rensaResult.chains = 1 + additionalChain;
    rensaResult.score = 0;
    rensaResult.frames = 0;

    RensaTracker tracker(&trackResult);
    simulateWithTracker(rensaResult, tracker);
}

template<typename Tracker>
inline void BasicField::simulateWithTracker(BasicRensaResult& rensaResult, Tracker& tracker)
{
    int minHeights[MAP_WIDTH] = { 100, 1, 1, 1, 1, 1, 1, 100 };

    while (vanish(rensaResult.chains, &rensaResult.score, minHeights, tracker)) {
        rensaResult.frames += dropAfterVanish(minHeights, tracker);
        rensaResult.frames += FRAMES_AFTER_VANISH;
        rensaResult.chains += 1;
    }

    rensaResult.chains -= 1;
}

std::string BasicField::debugOutput() const
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

