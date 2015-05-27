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
    BitField() {}
    explicit BitField(const PlainField&);
    explicit BitField(const std::string&);

    FieldBits bits(PuyoColor c) const;

    PuyoColor color(int x, int y) const;
    bool isColor(int x, int y, PuyoColor c) const { return bits(c).get(x, y); }
    bool isEmpty(int x, int y) const { return !(m_[0] | m_[1] | m_[2]).get(x, y); }
    void setColor(int x, int y, PuyoColor c);

    bool isZenkeshi() const { return FieldBits(m_[0] | m_[1] | m_[2]).isEmpty(); }

    bool isConnectedPuyo(int x, int y) const;
    int countConnectedPuyos(int x, int y) const;
    int countConnectedPuyos(int x, int y, FieldBits* checked) const;
    int countConnectedPuyosMax4(int x, int y) const;
    bool hasEmptyNeighbor(int x, int y) const;

    // TODO(mayah): This should be removed. This is for barkward compatibility.
    Position* fillSameColorPosition(int x, int y, PuyoColor c, Position* positionQueueHead, FieldBits* checked) const;

    RensaResult simulate();

private:
    friend class BitFieldTest;

    // Vanishes puyos. Returns score. Erased puyos are put |erased|.
    int vanish(int nthChain, FieldBits* erased);
    // Drops puyos. Returns max drops.
    int drop(FieldBits erased);

    FieldBits m_[3];
};

inline
FieldBits BitField::bits(PuyoColor c) const
{
    switch (c) {
    case PuyoColor::EMPTY:  // = 0
        return FieldBits();
    case PuyoColor::OJAMA:  // = 1  001
        return FieldBits(_mm_andnot_si128(m_[2].xmm(), _mm_andnot_si128(m_[1].xmm(), m_[0].xmm())));
    case PuyoColor::WALL:
        return FieldBits();
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

#endif // CORE_BIT_FIELD_H_
