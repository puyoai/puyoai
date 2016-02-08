#ifndef CORE_BIT_FIELD_H_
#define CORE_BIT_FIELD_H_

#include <string>

#include <glog/logging.h>

#include "base/base.h"
#include "base/sse.h"
#include "core/field_bits.h"
#include "core/frame.h"
#include "core/puyo_color.h"
#include "core/rensa_result.h"
#include "core/rensa_tracker.h"
#include "core/score.h"

class PlainField;
struct Position;

// BitsField is a field implementation that uses FieldBits.
class BitField {
public:
    struct SimulationContext {
        explicit SimulationContext(int currentChain = 1) : currentChain(currentChain) {}

        int currentChain = 1;
    };

    BitField();
    explicit BitField(const PlainField&);
    explicit BitField(const std::string&);

    FieldBits bits(PuyoColor c) const;
    FieldBits normalColorBits() const { return m_[2]; }
    FieldBits field13Bits() const { return (m_[0] | m_[1] | m_[2]).maskedField13(); }

    FieldBits differentBits(const BitField& bf) const {
        return (m_[0] ^ bf.m_[0]) | (m_[1] ^ bf.m_[1]) | (m_[2] ^ bf.m_[2]);
    }

    PuyoColor color(int x, int y) const;
    bool isColor(int x, int y, PuyoColor c) const { return bits(c).get(x, y); }
    bool isNormalColor(int x, int y) const { return m_[2].get(x, y); }
    bool isEmpty(int x, int y) const { return !(m_[0] | m_[1] | m_[2]).get(x, y); }

    void setColor(int x, int y, PuyoColor c);
    void setColorAll(FieldBits, PuyoColor);
    void setColorAllIfEmpty(FieldBits, PuyoColor);

    bool isZenkeshi() const { return FieldBits(m_[0] | m_[1] | m_[2]).maskedField13().isEmpty(); }

    bool isConnectedPuyo(int x, int y) const { return isConnectedPuyo(x, y, color(x, y)); }
    bool isConnectedPuyo(int x, int y, PuyoColor c) const;
    int countConnectedPuyos(int x, int y) const { return countConnectedPuyos(x, y, color(x, y)); }
    int countConnectedPuyos(int x, int y, PuyoColor c) const;
    int countConnectedPuyos(int x, int y, FieldBits* checked) const { return countConnectedPuyos(x, y, color(x, y), checked); }
    int countConnectedPuyos(int x, int y, PuyoColor c, FieldBits* checked) const;
    int countConnectedPuyosMax4(int x, int y) const { return countConnectedPuyosMax4(x, y, color(x, y)); }
    int countConnectedPuyosMax4(int x, int y, PuyoColor c) const;
    bool hasEmptyNeighbor(int x, int y) const;

    void countConnection(int* count2, int* count3) const;

    // Returns true if there are floating puyos.
    bool hasFloatingPuyo() const;

    RensaResult simulate(int initialChain = 1);
    template<typename Tracker> NOINLINE_UNLESS_RELEASE RensaResult simulate(SimulationContext*, Tracker*);
    // Faster version of simulate(). Returns the number of chains.
    template<typename Tracker> int simulateFast(Tracker*);
    // Vanishes the connected puyos, and drop the puyos in the air. Score will be returned.
    template<typename Tracker> NOINLINE_UNLESS_RELEASE RensaStepResult vanishDrop(SimulationContext*, Tracker*);
    template<typename Tracker> bool vanishDropFast(SimulationContext*, Tracker*);

    // Caution: heights must be aligned to 16.
    void calculateHeight(int heights[FieldConstant::MAP_WIDTH]) const;

    std::string toString(char charIfEmpty = ' ') const;
    std::string toDebugString(char charIfEmpty = ' ') const;

    bool rensaWillOccur() const;

    // TODO(mayah): This should be removed. This is for barkward compatibility.
    Position* fillSameColorPosition(int x, int y, PuyoColor c, Position* positionQueueHead, FieldBits* checked) const;
    // TODO(mayah): This should be removed. This is for backward compatibility.
    int fillErasingPuyoPositions(Position* eraseQueue) const;
    FieldBits ignitionPuyoBits() const;

    size_t hash() const;

    friend bool operator==(const BitField&, const BitField&);
    friend std::ostream& operator<<(std::ostream&, const BitField&);

#if defined(__AVX2__) && defined(__BMI2__)
    // Faster version of simulate() that uses AVX2 instruction set.
    template<typename Tracker> RensaResult NOINLINE_UNLESS_RELEASE simulateAVX2(SimulationContext*, Tracker*);
    template<typename Tracker> int simulateFastAVX2(Tracker*);
    template<typename Tracker> RensaStepResult NOINLINE_UNLESS_RELEASE vanishDropAVX2(SimulationContext*, Tracker*);
    template<typename Tracker> bool vanishDropFastAVX2(SimulationContext*, Tracker*);
#endif

private:
    BitField escapeInvisible();
    void recoverInvisible(const BitField&);

    // Vanishes puyos. Returns score. Erased puyos are put |erased|.
    // Actually puyo won't be vanished in this method, though...
    template<typename Tracker>
    int vanish(int currentChain, FieldBits* erased, Tracker* tracker) const;
    // Vanishes puyos. Returns true if something will be erased.
    template<typename Tracker>
    bool vanishFast(int currentChain, FieldBits* erased, Tracker* tracker) const;

    // Drops puyos. Returns max drops.
    template<typename Tracker>
    int dropAfterVanish(FieldBits erased, Tracker* tracker);
    template<typename Tracker>
    void dropAfterVanishFast(FieldBits erased, Tracker* tracker);

#if defined(__AVX2__) && defined(__BMI2__)
    template<typename Tracker>
    int vanishAVX2(int currentChain, FieldBits* erased, Tracker* tracker) const;
    template<typename Tracker>
    bool vanishFastAVX2(int currentChain, FieldBits* erased, Tracker* tracker) const;
    template<typename Tracker>
    int dropAfterVanishAVX2(FieldBits erased, Tracker* tracker);
    template<typename Tracker>
    void dropAfterVanishFastAVX2(FieldBits erased, Tracker* tracker);
#endif

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
BitField BitField::escapeInvisible()
{
    BitField escaped;
    for (int i = 0; i < 3; ++i) {
        escaped.m_[i] = m_[i].notmask(FieldBits::FIELD_MASK_13);
        m_[i] = m_[i].mask(FieldBits::FIELD_MASK_13);
    }

    return escaped;
}

inline
void BitField::recoverInvisible(const BitField& bf)
{
    for (int i = 0; i < 3; ++i) {
        m_[i].setAll(bf.m_[i]);
    }
}

inline
PuyoColor BitField::color(int x, int y) const
{
    int b0 = m_[0].get(x, y) ? 1 : 0;
    int b1 = m_[1].get(x, y) ? 2 : 0;
    int b2 = m_[2].get(x, y) ? 4 : 0;

    return static_cast<PuyoColor>(b0 | b1 | b2);
}

inline
void BitField::setColor(int x, int y, PuyoColor c)
{
    int cc = static_cast<int>(c);
    for (int i = 0; i < 3; ++i) {
        if (cc & (1 << i))
            m_[i].set(x, y);
        else
            m_[i].unset(x, y);
    }
}

inline
RensaResult BitField::simulate(int initialChain)
{
    RensaNonTracker tracker;
    SimulationContext context(initialChain);
    return simulate(&context, &tracker);
}

inline
void BitField::setColorAll(FieldBits bits, PuyoColor c)
{
    for (int i = 0; i < 3; ++i) {
        if (static_cast<int>(c) & (1 << i)) {
            m_[i].setAll(bits);
        } else {
            m_[i].unsetAll(bits);
        }
    }
}

inline
void BitField::setColorAllIfEmpty(FieldBits bits, PuyoColor c)
{
    FieldBits nonEmpty = (m_[0] | m_[1] | m_[2]);
    bits = bits.notmask(nonEmpty);

    setColorAll(bits, c);
}

inline
bool BitField::isConnectedPuyo(int x, int y, PuyoColor c) const
{
    DCHECK_EQ(c, color(x, y));

    if (y > FieldConstant::HEIGHT)
        return false;

    FieldBits colorBits = bits(c).maskedField12();
    FieldBits single(x, y);
    return !single.expandEdge().mask(colorBits).notmask(single).isEmpty();
}

inline
int BitField::countConnectedPuyos(int x, int y, PuyoColor c) const
{
    DCHECK_EQ(c, color(x, y));

    if (y > FieldConstant::HEIGHT)
        return 0;

    FieldBits colorBits = bits(c).maskedField12();
    return FieldBits(x, y).expand(colorBits).popcount();
}

inline
int BitField::countConnectedPuyos(int x, int y, PuyoColor c, FieldBits* checked) const
{
    DCHECK_EQ(c, color(x, y));

    if (y > FieldConstant::HEIGHT)
        return false;

    FieldBits colorBits = bits(c).maskedField12();
    FieldBits connected = FieldBits(x, y).expand(colorBits);
    checked->setAll(connected);
    return connected.popcount();
}

inline
int BitField::countConnectedPuyosMax4(int x, int y, PuyoColor c) const
{
    DCHECK_EQ(c, color(x, y));

    if (y > FieldConstant::HEIGHT)
        return false;

    FieldBits colorBits = bits(c).maskedField12();
    return FieldBits(x, y).expand4(colorBits).popcount();
}

inline
void BitField::calculateHeight(int heights[FieldConstant::MAP_WIDTH]) const
{
    const __m128i zero = _mm_setzero_si128();
    __m128i whole = (m_[0] | m_[1] | m_[2]).maskedField13();

   __m128i count = sse::mm_popcnt_epi16(whole);

   _mm_store_si128(reinterpret_cast<__m128i*>(heights), _mm_unpacklo_epi16(count, zero));
   _mm_store_si128(reinterpret_cast<__m128i*>(heights + 4), _mm_unpackhi_epi16(count, zero));
}

namespace std {

template<>
struct hash<BitField>
{
    size_t operator()(const BitField& bf) const
    {
        return bf.hash();
    }
};

}

#include "bit_field_inl.h"

#if defined(__AVX2__) && defined(__BMI2__)
#include "bit_field_avx2_inl.h"
#endif

#endif // CORE_BIT_FIELD_H_
