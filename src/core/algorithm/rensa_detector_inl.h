#ifndef CORE_ALGORITHM_RENSA_DETECTOR_INL_H_
#define CORE_ALGORITHM_RENSA_DETECTOR_INL_H_

// rensa_detector_inl.h contains several method implementation of RensaDetector.
// All methods should be inline or template.

#include "core/column_puyo_list.h"
#include "core/core_field.h"

// static
template<typename Callback>
void RensaDetector::complementKeyPuyos(const CoreField& originalField,
                                       const RensaDetectorStrategy& strategy,
                                       int maxKeyPuyos,
                                       Callback callback)
{
    CoreField cf(originalField);
    ColumnPuyoList cpl;
    complementKeyPuyosInternal(cf, cpl, strategy, 1, maxKeyPuyos, callback);
}

// To avoid copying CoreField and ColumnPuyoList, we use one instance.
// Be careful not to break loop invariant.
// static
template<typename Callback>
void RensaDetector::complementKeyPuyosInternal(CoreField& currentField,
                                               ColumnPuyoList& currentKeyPuyos,
                                               const RensaDetectorStrategy& strategy,
                                               int leftX,
                                               int restAdded,
                                               Callback callback)
{
    callback(const_cast<const CoreField&>(currentField),
             const_cast<const ColumnPuyoList&>(currentKeyPuyos));

    if (restAdded <= 0)
        return;

    for (int x = leftX; x <= FieldConstant::WIDTH; ++x) {
        if (currentField.height(x) >= strategy.maxKeyPuyoHeight())
            continue;

        for (PuyoColor c : NORMAL_PUYO_COLORS) {
            if (!currentField.dropPuyoOn(x, c))
                continue;
            if (!currentKeyPuyos.add(x, c)) {
                currentField.removePuyoFrom(x);
                continue;
            }

            if (currentField.countConnectedPuyosMax4(x, currentField.height(x)) < 4) {
                complementKeyPuyosInternal(currentField, currentKeyPuyos, strategy,
                                           x, restAdded - 1, callback);
            }

            currentField.removePuyoFrom(x);
            currentKeyPuyos.removeTopFrom(x);
        }
    }
}

#endif // CORE_ALGORITHM_RENSA_DETECTOR_INL_H_
