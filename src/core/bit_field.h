#ifndef CORE_BITS_FIELD_H_
#define CORE_BITS_FIELD_H_

#include <string>

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
    template<typename Tracker>
    RensaResult simulate(Tracker*);

    std::string toString(char charIfEmpty = ' ') const;

    friend bool operator==(const BitField&, const BitField&);
    friend std::ostream& operator<<(std::ostream&, const BitField&);

private:
    friend class BitFieldTest;

    BitField escapeInvisible();
    void recoverInvisible(const BitField&);

    // Vanishes puyos. Returns score. Erased puyos are put |erased|.
    // Actually puyo won't be vanished in this method, though...
    template<typename Tracker>
    int vanishForSimulation(int nthChain, FieldBits* erased, Tracker* tracker);
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
RensaResult BitField::simulate()
{
    RensaNonTracker tracker;
    return simulate(&tracker);
}

template<typename Tracker>
RensaResult BitField::simulate(Tracker* tracker)
{
    BitField escaped = escapeInvisible();

    int currentChain = 1;
    int score = 0;
    int frames = 0;
    int nthChainScore;
    bool quick = false;
    FieldBits erased;

    while ((nthChainScore = vanishForSimulation(currentChain, &erased, tracker)) > 0) {
        currentChain += 1;
        score += nthChainScore;
        frames += FRAMES_VANISH_ANIMATION;
        int maxDrops = dropAfterVanish(erased);
        if (maxDrops > 0) {
            frames += FRAMES_TO_DROP_FAST[maxDrops] + FRAMES_GROUNDING;
        } else {
            quick = true;
        }
    }

    recoverInvisible(escaped);
    return RensaResult(currentChain - 1, score, frames, quick);
}

template<typename Tracker>
int BitField::vanishForSimulation(int currentChain, FieldBits* erased, Tracker* tracker)
{
    int numErasedPuyos = 0;
    int numColors = 0;
    int longBonusCoef = 0;

    *erased = FieldBits();

    for (PuyoColor c : NORMAL_PUYO_COLORS) {
        FieldBits mask = bits(c).maskedField12();
        FieldBits seed = mask.vanishingSeed();

        if (seed.isEmpty())
            continue;

        ++numColors;

        // fast path. In most cases, 8>= puyos won't be erased.
        // When 7<= puyos are erased, it won't be separated.
        {
            FieldBits expanded = seed.expand(mask);
            int popcount = expanded.popcount();
            if (popcount <= 7) {
                numErasedPuyos += popcount;
                longBonusCoef += longBonus(popcount);
                erased->setAll(expanded);
                mask.unsetAll(expanded);
                continue;
            }
        }

        // slow path...
        seed.iterateBitWithMasking([&](FieldBits x) -> FieldBits {
            if (mask.testz(x))
                return x;

            FieldBits expanded = x.expand(mask);
            int count = expanded.popcount();
            numErasedPuyos += count;
            longBonusCoef += longBonus(count);
            erased->setAll(expanded);
            mask.unsetAll(expanded);
            return expanded;
        });
    }

    if (numColors == 0)
        return 0;

    int rensaBonusCoef = calculateRensaBonusCoef(chainBonus(currentChain), longBonusCoef, colorBonus(numColors));

    // Removes ojama.
    FieldBits ojamaErased(erased->expandEdge().mask(bits(PuyoColor::OJAMA).maskedField12()));
    tracker->track(currentChain, numErasedPuyos, rensaBonusCoef, *erased, ojamaErased);

    erased->setAll(ojamaErased);
    return 10 * numErasedPuyos * rensaBonusCoef;
}

#endif // CORE_BIT_FIELD_H_
