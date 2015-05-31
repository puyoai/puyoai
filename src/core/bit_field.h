#ifndef CORE_BITS_FIELD_H_
#define CORE_BITS_FIELD_H_

#include <string>

#include "base/base.h"
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

    PuyoColor color(int x, int y) const;
    bool isColor(int x, int y, PuyoColor c) const { return bits(c).get(x, y); }
    bool isNormalColor(int x, int y) const { return m_[2].get(x, y); }
    bool isEmpty(int x, int y) const { return !(m_[0] | m_[1] | m_[2]).get(x, y); }
    void setColor(int x, int y, PuyoColor c);

    bool isZenkeshi() const { return FieldBits(m_[0] | m_[1] | m_[2]).maskedField13().isEmpty(); }

    bool isConnectedPuyo(int x, int y) const;
    int countConnectedPuyos(int x, int y) const;
    int countConnectedPuyos(int x, int y, FieldBits* checked) const;
    int countConnectedPuyosMax4(int x, int y) const;
    bool hasEmptyNeighbor(int x, int y) const;

    void countConnection(int* count2, int* count3) const;

    RensaResult simulate(int initialChain = 1);
    RensaResult simulate(SimulationContext*);
    template<typename Tracker> RensaResult simulate(Tracker*);
    template<typename Tracker> RensaResult simulate(SimulationContext*, Tracker*) NOINLINE_UNLESS_RELEASE;

    // Vanishes the connected puyos, and drop the puyos in the air. Score will be returned.
    RensaStepResult vanishDrop(SimulationContext*);
    // Vanishes the connected puyos with Tracker.
    template<typename Tracker>
    RensaStepResult vanishDrop(SimulationContext*, Tracker*) NOINLINE_UNLESS_RELEASE;

    void calculateHeight(std::uint16_t heights[FieldConstant::MAP_WIDTH]) const;

    std::string toString(char charIfEmpty = ' ') const;
    std::string toDebugString(char charIfEmpty = ' ') const;

    int vanish(int currentChain);
    void drop();

    bool rensaWillOccur() const;

    // TODO(mayah): This should be removed. This is for barkward compatibility.
    Position* fillSameColorPosition(int x, int y, PuyoColor c, Position* positionQueueHead, FieldBits* checked) const;
    // TODO(mayah): This should be removed. This is for backward compatibility.
    int fillErasingPuyoPositions(Position* eraseQueue) const;

    friend bool operator==(const BitField&, const BitField&);
    friend std::ostream& operator<<(std::ostream&, const BitField&);

private:
    BitField escapeInvisible();
    void recoverInvisible(const BitField&);

    // Vanishes puyos. Returns score. Erased puyos are put |erased|.
    // Actually puyo won't be vanished in this method, though...
    template<typename Tracker>
    int vanishForSimulation(int nthChain, FieldBits* erased, Tracker* tracker) const;
    // Drops puyos. Returns max drops.
    template<typename Tracker>
    int dropAfterVanish(FieldBits erased, Tracker* tracker);

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
    const FieldBits mask = _mm_set_epi16(0, 0x3FFE, 0x3FFE, 0x3FFE, 0x3FFE, 0x3FFE, 0x3FFE, 0);
    BitField escaped;
    for (int i = 0; i < 3; ++i) {
        escaped.m_[i] = m_[i].notmask(mask);
        m_[i] = m_[i].mask(mask);
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
RensaResult BitField::simulate(SimulationContext* context)
{
    RensaNonTracker tracker;
    return simulate(context, &tracker);
}

template<typename Tracker>
RensaResult BitField::simulate(Tracker* tracker)
{
    SimulationContext context(1);
    return simulate(&context, tracker);
}

inline
RensaStepResult BitField::vanishDrop(BitField::SimulationContext* context)
{
    RensaNonTracker tracker;
    return vanishDrop(context, &tracker);
}

inline
void BitField::calculateHeight(std::uint16_t heights[FieldConstant::MAP_WIDTH]) const
{
    __m128i whole = (m_[0] | m_[1] | m_[2]).maskedField13();

    const __m128i mask4 = _mm_set1_epi8(0x0F);
    const __m128i lookup = _mm_setr_epi8(0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4);

   __m128i low = _mm_and_si128(mask4, whole);
   __m128i high = _mm_and_si128(mask4, _mm_srli_epi16(whole, 4));

   __m128i lowCount = _mm_shuffle_epi8(lookup, low);
   __m128i highCount = _mm_shuffle_epi8(lookup, high);
   __m128i count8 = _mm_add_epi8(lowCount, highCount);

   __m128i count16 = _mm_add_epi8(count8, _mm_srli_epi16(count8, 8));
   __m128i count = _mm_and_si128(count16, _mm_set1_epi16(0x0F));
   _mm_store_si128(reinterpret_cast<__m128i*>(heights), count);
}


#endif // CORE_BIT_FIELD_H_
