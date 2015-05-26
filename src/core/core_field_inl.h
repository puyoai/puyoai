#ifndef CORE_CORE_FIELD_INL_H_
#define CORE_CORE_FIELD_INL_H_

#include "core/column_puyo_list.h"
#include "core/field_checker.h"
#include "core/frame.h"
#include "core/rensa_tracker.h"
#include "core/score.h"

// core_field_inl.h contains several method implementation of CoreField.
// All methods should be inline or template.

inline
RensaResult CoreField::simulate(int initialChain)
{
    SimulationContext context(initialChain);
    RensaNonTracker tracker;
    return simulate(&context, &tracker);
}

inline
RensaResult CoreField::simulate(SimulationContext* context)
{
    RensaNonTracker tracker;
    return simulate(context, &tracker);
}

template<typename Tracker>
RensaResult CoreField::simulate(Tracker* tracker)
{
    SimulationContext context;
    return simulate(&context, tracker);
}

template<typename Tracker>
inline RensaResult CoreField::simulate(SimulationContext* context, Tracker* tracker)
{
    int score = 0;
    int frames = 0;

    int nthChainScore;
    bool quick = false;
    while ((nthChainScore = vanish(context, tracker)) > 0) {
        context->currentChain += 1;
        score += nthChainScore;
        frames += FRAMES_VANISH_ANIMATION;
        int maxDrops = dropAfterVanish(context, tracker);
        if (maxDrops > 0) {
            DCHECK(maxDrops < 14);
            frames += FRAMES_TO_DROP_FAST[maxDrops] + FRAMES_GROUNDING;
        } else {
            quick = true;
        }
    }

    return RensaResult(context->currentChain - 1, score, frames, quick);
}

inline
RensaStepResult CoreField::vanishDrop(SimulationContext* context)
{
    RensaNonTracker tracker;
    return vanishDrop(context, &tracker);
}

template<typename Tracker>
RensaStepResult CoreField::vanishDrop(SimulationContext* context, Tracker* tracker)
{
    int score = vanish(context, tracker);
    int maxDrops = 0;
    int frames = FRAMES_VANISH_ANIMATION;
    bool quick = false;
    if (score > 0)
        maxDrops = dropAfterVanish(context, tracker);

    if (maxDrops > 0) {
        DCHECK(maxDrops < 14);
        frames += FRAMES_TO_DROP_FAST[maxDrops] + FRAMES_GROUNDING;
    } else {
        quick = true;
    }

    return RensaStepResult(score, frames, quick);
}

template<typename Tracker>
int CoreField::vanish(SimulationContext* context, Tracker* tracker)
{
    FieldChecker checked;
    Position eraseQueue[WIDTH * HEIGHT]; // All the positions of erased puyos will be stored here.
    Position* eraseQueueHead = eraseQueue;

    bool usedColors[NUM_PUYO_COLORS] {};
    int numUsedColors = 0;
    int longBonusCoef = 0;

    for (int x = 1; x <= WIDTH; ++x) {
        int maxHeight = height(x);
        for (int y = context->minHeights[x]; y <= maxHeight; ++y) {
            DCHECK_NE(color(x, y), PuyoColor::EMPTY)
                << x << ' ' << y << ' ' << toChar(color(x, y)) << '\n'
                << toDebugString();

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
    eraseQueuedPuyos(context, eraseQueue, eraseQueueHead, tracker);

    int rensaBonusCoef = calculateRensaBonusCoef(chainBonus(context->currentChain), longBonusCoef, colorBonus(numUsedColors));
    tracker->nthChainDone(context->currentChain, numErasedPuyos, rensaBonusCoef);
    return 10 * numErasedPuyos * rensaBonusCoef;
}

template<typename Tracker>
void CoreField::eraseQueuedPuyos(SimulationContext* context, Position* eraseQueue, Position* eraseQueueHead, Tracker* tracker)
{
    DCHECK(tracker);

    context->updateFromField(*this);

    for (Position* head = eraseQueue; head != eraseQueueHead; ++head) {
        int x = head->x;
        int y = head->y;

        unsafeSet(x, y, PuyoColor::EMPTY);
        tracker->colorPuyoIsVanished(x, y, context->currentChain);
        context->minHeights[x] = std::min(context->minHeights[x], y);

        // Check OJAMA puyos erased
        if (color(x + 1, y) == PuyoColor::OJAMA) {
            unsafeSet(x + 1, y, PuyoColor::EMPTY);
            tracker->ojamaPuyoIsVanished(x + 1, y, context->currentChain);
            context->minHeights[x + 1] = std::min(context->minHeights[x + 1], y);
        }

        if (color(x - 1, y) == PuyoColor::OJAMA) {
            unsafeSet(x - 1, y, PuyoColor::EMPTY);
            tracker->ojamaPuyoIsVanished(x - 1, y, context->currentChain);
            context->minHeights[x - 1] = std::min(context->minHeights[x - 1], y);
        }

        // We don't need to update minHeights here.
        if (color(x, y + 1) == PuyoColor::OJAMA && y + 1 <= HEIGHT) {
            unsafeSet(x, y + 1, PuyoColor::EMPTY);
            tracker->ojamaPuyoIsVanished(x, y + 1, context->currentChain);
        }

        if (color(x, y - 1) == PuyoColor::OJAMA) {
            unsafeSet(x, y - 1, PuyoColor::EMPTY);
            tracker->ojamaPuyoIsVanished(x, y - 1, context->currentChain);
            context->minHeights[x] = std::min(context->minHeights[x], y - 1);
        }
    }
}

template<typename Tracker>
int CoreField::dropAfterVanish(SimulationContext* context, Tracker* tracker)
{
    DCHECK(tracker);

    int maxDrops = 0;
    for (int x = 1; x <= WIDTH; x++) {
        int writeAt = context->minHeights[x];
        if (writeAt >= 14)
            continue;

        int maxHeight = height(x);
        heights_[x] = writeAt - 1;

        DCHECK_EQ(color(x, writeAt), PuyoColor::EMPTY) << writeAt << ' ' << toChar(color(x, writeAt));
        for (int y = writeAt + 1; y <= maxHeight; ++y) {
            if (color(x, y) == PuyoColor::EMPTY)
                continue;

            maxDrops = std::max(maxDrops, y - writeAt);
            unsafeSet(x, writeAt, color(x, y));
            unsafeSet(x, y, PuyoColor::EMPTY);
            heights_[x] = writeAt;
            tracker->puyoIsDropped(x, y, writeAt++);
        }
    }

    return maxDrops;
}

inline
void CoreField::removePuyoFrom(int x)
{
    DCHECK_GE(height(x), 1);
    unsafeSet(x, heights_[x]--, PuyoColor::EMPTY);
}

inline
void CoreField::removePuyoFrom(int x, int n)
{
    DCHECK_GE(height(x), n);
    for (int i = 0; i < n; ++i) {
        unsafeSet(x, heights_[x]--, PuyoColor::EMPTY);
    }
}

inline
void CoreField::remove(const ColumnPuyoList& cpl)
{
    for (int x = 1; x <= 6; ++x) {
        for (int i = cpl.sizeOn(x); i > 0; --i) {
            DCHECK_EQ(color(x, height(x)), cpl.get(x, i - 1));
            removePuyoFrom(x);
        }
    }
}

#endif
