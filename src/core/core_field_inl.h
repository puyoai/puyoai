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
CoreField::CoreField(const BitField& f) :
    field_(f)
{
    f.calculateHeight(heights_);
}

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
    RensaResult result = field_.simulate(context, tracker);
    field_.calculateHeight(heights_);
    return result;
}

inline
RensaStepResult CoreField::vanishDrop()
{
    RensaNonTracker tracker;
    return vanishDrop(&tracker);
}

inline
RensaStepResult CoreField::vanishDrop(SimulationContext* context)
{
    RensaNonTracker tracker;
    return vanishDrop(context, &tracker);
}

template<typename Tracker>
RensaStepResult CoreField::vanishDrop(Tracker* tracker)
{
    SimulationContext context;
    return vanishDrop(&context, tracker);
}

template<typename Tracker>
RensaStepResult CoreField::vanishDrop(SimulationContext* context, Tracker* tracker)
{
    RensaStepResult result = field_.vanishDrop(context, tracker);
    field_.calculateHeight(heights_);
    return result;
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
