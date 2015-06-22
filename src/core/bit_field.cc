#include "core/bit_field.h"

#include <smmintrin.h>
#include <glog/logging.h>

#include <sstream>

#include "core/frame.h"
#include "core/plain_field.h"
#include "core/position.h"
#include "core/score.h"

using namespace std;

BitField::BitField()
{
    // Sets WALL
    m_[1].setAll(_mm_set_epi16(0xFFFF, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0x8001, 0xFFFF));
}

BitField::BitField(const PlainField& pf) : BitField()
{
    __m128i m1 = _mm_load_si128(reinterpret_cast<const __m128i*>(pf.column(1)));
    __m128i m2 = _mm_load_si128(reinterpret_cast<const __m128i*>(pf.column(2)));
    __m128i m3 = _mm_load_si128(reinterpret_cast<const __m128i*>(pf.column(3)));
    __m128i m4 = _mm_load_si128(reinterpret_cast<const __m128i*>(pf.column(4)));
    __m128i m5 = _mm_load_si128(reinterpret_cast<const __m128i*>(pf.column(5)));
    __m128i m6 = _mm_load_si128(reinterpret_cast<const __m128i*>(pf.column(6)));

    for (int i = 0; i < NUM_PUYO_COLORS; ++i) {
        PuyoColor c = static_cast<PuyoColor>(i);
        if (c == PuyoColor::EMPTY || c == PuyoColor::WALL) {
            continue;
        }

        __m128i mask = _mm_set1_epi8(static_cast<char>(c));

        union {
            __m128i m;
            std::int16_t s[8];
        } xmm;

        xmm.s[0] = 0;
        xmm.s[1] = _mm_movemask_epi8(_mm_cmpeq_epi8(m1, mask));
        xmm.s[2] = _mm_movemask_epi8(_mm_cmpeq_epi8(m2, mask));
        xmm.s[3] = _mm_movemask_epi8(_mm_cmpeq_epi8(m3, mask));
        xmm.s[4] = _mm_movemask_epi8(_mm_cmpeq_epi8(m4, mask));
        xmm.s[5] = _mm_movemask_epi8(_mm_cmpeq_epi8(m5, mask));
        xmm.s[6] = _mm_movemask_epi8(_mm_cmpeq_epi8(m6, mask));
        xmm.s[7] = 0;

        if (ordinal(c) & 1)
            m_[0].setAll(FieldBits(xmm.m));
        if (ordinal(c) & 2)
            m_[1].setAll(FieldBits(xmm.m));
        if (ordinal(c) & 4)
            m_[2].setAll(FieldBits(xmm.m));
    }
}

BitField::BitField(const string& str) : BitField()
{
    int counter = 0;
    for (int i = str.length() - 1; i >= 0; --i) {
        int x = 6 - (counter % 6);
        int y = counter / 6 + 1;
        PuyoColor c = toPuyoColor(str[i]);
        setColor(x, y, c);
        counter++;
    }
}

bool BitField::isConnectedPuyo(int x, int y) const
{
    if (y > FieldConstant::HEIGHT)
        return false;

    FieldBits colorBits = bits(color(x, y)).maskedField12();
    FieldBits single(x, y);
    return !single.expandEdge().mask(colorBits).notmask(single).isEmpty();
}

int BitField::countConnectedPuyos(int x, int y) const
{
    if (y > FieldConstant::HEIGHT)
        return 0;

    FieldBits colorBits = bits(color(x, y)).maskedField12();
    return FieldBits(x, y).expand(colorBits).popcount();
}

int BitField::countConnectedPuyos(int x, int y, FieldBits* checked) const
{
    if (y > FieldConstant::HEIGHT)
        return false;

    FieldBits colorBits = bits(color(x, y)).maskedField12();
    FieldBits connected = FieldBits(x, y).expand(colorBits);
    checked->setAll(connected);
    return connected.popcount();
}

int BitField::countConnectedPuyosMax4(int x, int y) const
{
    if (y > FieldConstant::HEIGHT)
        return false;

    FieldBits colorBits = bits(color(x, y)).maskedField12();
    return FieldBits(x, y).expand4(colorBits).popcount();
}

bool BitField::hasEmptyNeighbor(int x, int y) const
{
    if (x + 1 <= 6 && isEmpty(x + 1, y))
        return true;
    if (x - 1 >= 1 && isEmpty(x - 1, y))
        return true;
    if (y - 1 >= 1 && isEmpty(x, y - 1))
        return true;
    if (y + 1 <= 12 && isEmpty(x, y + 1))
        return true;
    return false;
}

void BitField::countConnection(int* count2, int* count3) const
{
    *count2 = *count3 = 0;

    for (PuyoColor c : NORMAL_PUYO_COLORS) {
        int cnt2, cnt3;
        bits(c).countConnection(&cnt2, &cnt3);
        *count2 += cnt2;
        *count3 += cnt3;
    }
}

Position* BitField::fillSameColorPosition(int x, int y, PuyoColor c,
                                          Position* positionQueueHead, FieldBits* checked) const
{
    FieldBits bs = FieldBits(x, y).expand(bits(c).maskedField12());
    checked->setAll(bs);
    int len = bs.toPositions(positionQueueHead);
    return positionQueueHead + len;
}

FieldBits BitField::ignitionPuyoBits() const
{
    FieldBits bits;
    RensaNonTracker tracker;
    (void)vanishForSimulation(1, &bits, &tracker);

    return bits;
}

int BitField::fillErasingPuyoPositions(Position* eraseQueue) const
{
    FieldBits bits;
    RensaNonTracker tracker;
    int score = vanishForSimulation(1, &bits, &tracker);
    if (score == 0)
        return 0;

    return bits.toPositions(eraseQueue);
}

bool BitField::rensaWillOccur() const
{
    for (PuyoColor c : NORMAL_PUYO_COLORS) {
        FieldBits mask = bits(c).maskedField12();
        FieldBits seed = mask.vanishingSeed();

        if (!seed.isEmpty())
            return true;
    }

    return false;
}

int BitField::vanish(int currentChain)
{
    FieldBits erased;
    RensaNonTracker tracker;
    int score = vanishForSimulation(currentChain, &erased, &tracker);
    for (auto& m : m_)
        m.unsetAll(erased);
    return score;
}

void BitField::drop()
{
    // TODO(mayah): slow!
    for (int x = 1; x <= 6; ++x) {
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

std::string BitField::toString(char charIfEmpty) const
{
    ostringstream ss;
    for (int y = 14; y >= 1; --y) {
        for (int x = 1; x <= FieldConstant::WIDTH; ++x) {
            ss << toChar(color(x, y), charIfEmpty);
        }
    }

    return ss.str();
}

std::string BitField::toDebugString(char charIfEmpty) const
{
    ostringstream ss;
    for (int y = FieldConstant::MAP_HEIGHT - 1; y >= 0; --y) {
        for (int x = 0; x < FieldConstant::MAP_WIDTH; ++x) {
            ss << toChar(color(x, y), charIfEmpty);
        }
        ss << '\n';
    }

    return ss.str();
}

bool operator==(const BitField& lhs, const BitField& rhs)
{
    for (int i = 0; i < 3; ++i)
        if (lhs.m_[i] != rhs.m_[i])
            return false;
    return true;
}

std::ostream& operator<<(std::ostream& os, const BitField& bf)
{
    return os << bf.toString();
}
