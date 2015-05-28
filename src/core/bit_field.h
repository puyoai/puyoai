#ifndef CORE_BITS_FIELD_H_
#define CORE_BITS_FIELD_H_

#include <string>

#include "core/field_bits.h"
#include "core/puyo_color.h"
#include "core/rensa_result.h"

class PlainField;
struct Position;

// BitsField is a field implementation that uses FieldBits.
class BitField {
public:
    BitField();
    explicit BitField(const PlainField&);
    explicit BitField(const std::string&);

    FieldBits bits(PuyoColor c) const;

    PuyoColor color(int x, int y) const;
    bool isColor(int x, int y, PuyoColor c) const { return bits(c).get(x, y); }
    bool isEmpty(int x, int y) const { return !(m_[0] | m_[1] | m_[2]).get(x, y); }
    void setColor(int x, int y, PuyoColor c);

    bool isZenkeshi() const { return FieldBits(m_[0] | m_[1] | m_[2]).maskedField13().isEmpty(); }

    bool isConnectedPuyo(int x, int y) const;
    int countConnectedPuyos(int x, int y) const;
    int countConnectedPuyos(int x, int y, FieldBits* checked) const;
    int countConnectedPuyosMax4(int x, int y) const;
    bool hasEmptyNeighbor(int x, int y) const;

    int vanish(int currentChain);
    void drop();

    // TODO(mayah): This should be removed. This is for barkward compatibility.
    Position* fillSameColorPosition(int x, int y, PuyoColor c, Position* positionQueueHead, FieldBits* checked) const;

    RensaResult simulate();

    std::string toString(char charIfEmpty = ' ') const;

    friend bool operator==(const BitField&, const BitField&);
    friend std::ostream& operator<<(std::ostream&, const BitField&);

private:
    friend class BitFieldTest;

    BitField escapeUnvisible();
    void recoverUnvisible(const BitField&);

    // Vanishes puyos. Returns score. Erased puyos are put |erased|.
    // Actually puyo won't be vanished in this method, though...
    int vanishForSimulation(int nthChain, FieldBits* erased);
    // Drops puyos. Returns max drops.
    int dropAfterVanish(FieldBits erased);

    FieldBits m_[3];
};

inline
FieldBits BitField::bits(PuyoColor c) const
{
    const __m128i zero = _mm_setzero_si128();

    switch (c) {
    case PuyoColor::EMPTY:  // = 0  000
        return (m_[0] | m_[1] | m_[2]) ^ _mm_cmpeq_epi8(zero, zero);
    case PuyoColor::OJAMA:  // = 1  001
        return FieldBits(_mm_andnot_si128(m_[2].xmm(), _mm_andnot_si128(m_[1].xmm(), m_[0].xmm())));
    case PuyoColor::WALL:   // = 2  010
        return FieldBits(_mm_andnot_si128(m_[2].xmm(), _mm_andnot_si128(m_[0].xmm(), m_[1].xmm())));
    case PuyoColor::IRON:   // = 3  011
        return FieldBits(_mm_andnot_si128(m_[2].xmm(), _mm_and_si128(m_[1].xmm(), m_[0].xmm())));
    case PuyoColor::RED:    // = 4  100
        return FieldBits(_mm_andnot_si128(m_[0].xmm(), _mm_andnot_si128(m_[1].xmm(), m_[2].xmm())));
    case PuyoColor::BLUE:   // = 5  101
        return FieldBits(_mm_and_si128(m_[0].xmm(), _mm_andnot_si128(m_[1].xmm(), m_[2].xmm())));
    case PuyoColor::YELLOW: // = 6  110
        return FieldBits(_mm_andnot_si128(m_[0].xmm(), _mm_and_si128(m_[1].xmm(), m_[2].xmm())));
    case PuyoColor::GREEN:  // = 7  111
        return FieldBits(_mm_and_si128(m_[0].xmm(), _mm_and_si128(m_[1].xmm(), m_[2].xmm())));
    }

    CHECK(false);
    return FieldBits();
}

inline
BitField BitField::escapeUnvisible()
{
    const FieldBits mask = _mm_set_epi16(0, 0x3FFE, 0x3FFE, 0x3FFE, 0x3FFE, 0x3FFE, 0x3FFE, 0);
    BitField escaped;
    for (int i = 0; i < 3; ++i) {
        escaped.m_[i] = m_[i].notmask(mask);
        m_[i] = m_[i].mask(mask);
    }

    return escaped;
}

inline
void BitField::recoverUnvisible(const BitField& bf)
{
    for (int i = 0; i < 3; ++i) {
        m_[i].setAll(bf.m_[i]);
    }
}

#endif // CORE_BIT_FIELD_H_
