#ifndef CORE_BIT_FIELD_INL_H_
#define CORE_BIT_FIELD_INL_H_

template<typename Tracker>
RensaResult BitField::simulate(SimulationContext* context, Tracker* tracker)
{
    BitField escaped = escapeInvisible();

    int score = 0;
    int frames = 0;
    int nthChainScore;
    bool quick = false;
    FieldBits erased;

    while ((nthChainScore = vanishForSimulation(context->currentChain, &erased, tracker)) > 0) {
        context->currentChain += 1;
        score += nthChainScore;
        frames += FRAMES_VANISH_ANIMATION;
        int maxDrops = dropAfterVanish(erased, tracker);
        if (maxDrops > 0) {
            frames += FRAMES_TO_DROP_FAST[maxDrops] + FRAMES_GROUNDING;
        } else {
            quick = true;
        }
    }

    recoverInvisible(escaped);
    return RensaResult(context->currentChain - 1, score, frames, quick);
}

template<typename Tracker>
RensaStepResult BitField::vanishDrop(SimulationContext* context, Tracker* tracker)
{
    BitField escaped = escapeInvisible();

    FieldBits erased;
    int score = vanishForSimulation(context->currentChain, &erased, tracker);
    int maxDrops = 0;
    int frames = FRAMES_VANISH_ANIMATION;
    bool quick = false;
    if (score > 0) {
        maxDrops = dropAfterVanish(erased, tracker);
        context->currentChain += 1;
    }

    if (maxDrops > 0) {
        DCHECK(maxDrops < 14);
        frames += FRAMES_TO_DROP_FAST[maxDrops] + FRAMES_GROUNDING;
    } else {
        quick = true;
    }

    recoverInvisible(escaped);
    return RensaStepResult(score, frames, quick);
}

template<typename Tracker>
int BitField::vanishForSimulation(int currentChain, FieldBits* erased, Tracker* tracker) const
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

        // fast path. In most cases, >= 8 puyos won't be erased.
        // When <= 7 puyos are erased, it won't be separated.
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

template<typename Tracker>
int BitField::dropAfterVanish(FieldBits erased, Tracker* tracker)
{
    // TODO(mayah): If we can use AVX2, we have PDEP, PEXT instruction.
    // It would be really useful to improve this method, I believe.

    // bits   = b15 .. b9 b8 b7 .. b0

    // Consider y = 8.
    // v1 = and ( b15 .. b9 b8 b7 .. b0,
    //          (   0 ..  0  0   1 ..  1) = 0-0 b7-b0
    // v2 = and ( b15 .. b9 b8 b7 .. b0,
    //          (   1 ..  1  0  0 ..  0) = b15-b9 0-0
    // v3 = v2 >> 1 = 0 b15-b9 0-0
    // v4 = v1 | v3 = 0 b15-b9 b7-b0
    // new bits = BLEND(bits, v4, blender)
    const __m128i zero = _mm_setzero_si128();
    const __m128i ones = _mm_cmpeq_epi8(zero, zero);
    const __m128i whole = _mm_andnot_si128(erased.xmm(),
        _mm_or_si128(m_[0].xmm(), _mm_or_si128(m_[1].xmm(), m_[2].xmm())));

    int wholeErased = erased.horizontalOr16();
    int maxY = 31 - __builtin_clz(wholeErased);
    int minY = __builtin_ctz(wholeErased);

    DCHECK(1 <= minY && minY <= maxY && maxY <= 12)
        << "minY=" << minY << ' ' << "maxY=" << maxY << std::endl << erased.toString();

    //        MSB      y      LSB
    // line  : 0 ... 0 1 0 ... 0
    // right : 0 ... 0 0 1 ... 1
    // left  : 1 ... 1 0 0 ... 0

    __m128i line = _mm_set1_epi16(1 << (maxY + 1));
    __m128i rightOnes = _mm_set1_epi16((1 << (maxY + 1)) - 1);
    __m128i leftOnes = _mm_set1_epi16(~((1 << ((maxY + 1) + 1)) - 1));

    __m128i dropAmount = zero;
    // For each line, -1 if there exists dropping puyo, 0 otherwise.
    __m128i exists = _mm_cmpeq_epi16(_mm_and_si128(line, whole), line);

    for (int y = maxY; y >= minY; --y) {
        line = _mm_srli_epi16(line, 1);
        rightOnes = _mm_srai_epi16(rightOnes, 1);
        leftOnes = _mm_srai_epi16(leftOnes, 1);   // needs arithmetic shift.

        // for each line, -1 if drop, 0 otherwise.
        __m128i blender = _mm_xor_si128(_mm_cmpeq_epi16(_mm_and_si128(line, erased.xmm()), zero), ones);

        tracker->trackDrop(blender, leftOnes, rightOnes);

        for (int i = 0; i < 3; ++i) {
            __m128i m = m_[i].xmm();
            __m128i v1 = _mm_and_si128(rightOnes, m);
            __m128i v2 = _mm_and_si128(leftOnes, m);
            __m128i v3 = _mm_srli_epi16(v2, 1);
            __m128i v4 = _mm_or_si128(v1, v3);
            // _mm_blend_epi16 takes const int for parameter, so let's use blendv_epi8.
            m = _mm_blendv_epi8(m, v4, blender);
            m_[i] = FieldBits(m);
        }

        // both blender and exists are -1, puyo will drop 1.
        // -(-1) = +1
        dropAmount = _mm_sub_epi16(dropAmount, _mm_and_si128(blender, exists));
        exists = _mm_or_si128(exists, _mm_cmpeq_epi16(_mm_and_si128(line, whole), line));
    }

    // We have _mm_minpos_epu16, but not _mm_maxpos_epu16. So, taking xor 1.
    int maxDropAmountNegative = _mm_cvtsi128_si32(_mm_minpos_epu16(_mm_xor_si128(ones, dropAmount)));
    return ~maxDropAmountNegative & 0xFF;
}

#endif // CORE_BIT_FIELD_INL_H_
