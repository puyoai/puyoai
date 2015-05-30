#include "core/plain_field.h"

#include <iomanip>
#include <sstream>

#include "core/field_checker.h"
#include "core/position.h"
#include "core/score.h"

using namespace std;

PlainField::PlainField()
{
    initialize();
}

PlainField::PlainField(const string& url)
{
    initialize();

    string prefix = "http://www.inosendo.com/puyo/rensim/??";
    int dataStartsAt = url.find(prefix) == 0 ? prefix.length() : 0;

    int counter = 0;
    for (int i = url.length() - 1; i >= dataStartsAt; --i) {
        int x = 6 - (counter % 6);
        int y = counter / 6 + 1;
        PuyoColor c = toPuyoColor(url[i]);
        setColor(x, y, c);
        counter++;
    }
}

void PlainField::initialize()
{
    // Initialize field information.
    for (int x = 0; x < MAP_WIDTH; ++x) {
        for (int y = 0; y < MAP_HEIGHT; ++y)
            setColor(x, y, PuyoColor::EMPTY);
    }

    for (int x = 0; x < MAP_WIDTH; ++x) {
        setColor(x, 0, PuyoColor::WALL);
        setColor(x, MAP_HEIGHT - 1, PuyoColor::WALL);
    }

    for (int y = 0; y < MAP_HEIGHT; ++y) {
        setColor(0, y, PuyoColor::WALL);
        setColor(MAP_WIDTH - 1, y, PuyoColor::WALL);
    }
}

bool PlainField::isZenkeshi() const
{
    for (int x = 1; x <= WIDTH; ++x) {
        for (int y = 1; y <= 13; ++y) {
            if (!isEmpty(x, y))
                return false;
        }
    }

    return true;
}

void PlainField::drop()
{
    for (int x = 1; x <= WIDTH; ++x) {
        int h = 1;
        for (int y = 1; y <= 13; ++y) {
            if (isEmpty(x, y))
                continue;
            setColor(x, h++, color(x, y));
        }
        for (; h <= 13; ++h) {
            setColor(x, h, PuyoColor::EMPTY);
        }
    }
}

Position* PlainField::fillSameColorPosition(int x, int y, PuyoColor c,
                                            Position* positionQueueHead, FieldChecker* checked) const
{
    DCHECK(!checked->get(x, y));

    if (FieldConstant::HEIGHT < y)
        return positionQueueHead;

    Position* writeHead = positionQueueHead;
    Position* readHead = positionQueueHead;

    *writeHead++ = Position(x, y);
    checked->set(x, y);

    while (readHead != writeHead) {
        Position p = *readHead++;

        if (color(p.x + 1, p.y) == c && !checked->get(p.x + 1, p.y)) {
            *writeHead++ = Position(p.x + 1, p.y);
            checked->set(p.x + 1, p.y);
        }
        if (color(p.x - 1, p.y) == c && !checked->get(p.x - 1, p.y)) {
            *writeHead++ = Position(p.x - 1, p.y);
            checked->set(p.x - 1, p.y);
        }
        if (color(p.x, p.y + 1) == c && !checked->get(p.x, p.y + 1) && p.y + 1 <= FieldConstant::HEIGHT) {
            *writeHead++ = Position(p.x, p.y + 1);
            checked->set(p.x, p.y + 1);
        }
        if (color(p.x, p.y - 1) == c && !checked->get(p.x, p.y - 1)) {
            *writeHead++ = Position(p.x, p.y - 1);
            checked->set(p.x, p.y - 1);
        }
    }

    return writeHead;
}

int PlainField::vanish(int currentChain)
{
    FieldChecker checked;
    Position eraseQueue[WIDTH * HEIGHT]; // All the positions of erased puyos will be stored here.
    Position* eraseQueueHead = eraseQueue;

    bool usedColors[NUM_PUYO_COLORS] {};
    int numUsedColors = 0;
    int longBonusCoef = 0;

    for (int x = 1; x <= WIDTH; ++x) {
        for (int y = 1; y <= HEIGHT; ++y) {
            if (isEmpty(x, y))
                continue;

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

    // --- Actually erase the Puyos to be vanished. We erase ojama here also.
    for (Position* head = eraseQueue; head != eraseQueueHead; ++head) {
        int x = head->x;
        int y = head->y;

        setColor(x, y, PuyoColor::EMPTY);

        // Check OJAMA puyos erased
        if (color(x + 1, y) == PuyoColor::OJAMA) {
            setColor(x + 1, y, PuyoColor::EMPTY);
        }

        if (color(x - 1, y) == PuyoColor::OJAMA) {
            setColor(x - 1, y, PuyoColor::EMPTY);
        }

        // We don't need to update minHeights here.
        if (color(x, y + 1) == PuyoColor::OJAMA && y + 1 <= HEIGHT) {
            setColor(x, y + 1, PuyoColor::EMPTY);
        }

        if (color(x, y - 1) == PuyoColor::OJAMA) {
            setColor(x, y - 1, PuyoColor::EMPTY);
        }
    }

    int rensaBonusCoef = calculateRensaBonusCoef(chainBonus(currentChain), longBonusCoef, colorBonus(numUsedColors));
    return 10 * numErasedPuyos * rensaBonusCoef;
}

int PlainField::countConnectedPuyosMax4(int x, int y) const
{
    if (y > FieldConstant::HEIGHT)
      return 0;

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

    return cnt;
}

int PlainField::countConnectedPuyos(int x, int y) const
{
    FieldChecker checked;
    return countConnectedPuyos(x, y, &checked);
}

int PlainField::countConnectedPuyos(int x, int y, FieldChecker* checked) const
{
    Position positions[WIDTH * HEIGHT];

    Position* filledHead = fillSameColorPosition(x, y, color(x, y), positions, checked);
    return filledHead - positions;
}

string PlainField::toString(char charIfEmpty) const
{
    ostringstream ss;
    for (int y = 14; y >= 1; --y) {
        for (int x = 1; x <= WIDTH; ++x) {
            ss << toChar(color(x, y), charIfEmpty);
        }
    }

    return ss.str();
}

string PlainField::toDebugString() const
{
    std::ostringstream s;
    for (int y = MAP_HEIGHT - 1; y >= 0; y--) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            s << toChar(color(x, y)) << ' ';
        }
        s << std::endl;
    }
    return s.str();
}

bool operator==(const PlainField& lhs, const PlainField& rhs)
{
    for (int x = 1; x <= FieldConstant::WIDTH; ++x) {
        for (int y = 1; y <= FieldConstant::HEIGHT; ++y) {
            if (lhs.color(x, y) != rhs.color(x, y))
                return false;
        }
    }

    return true;
}
