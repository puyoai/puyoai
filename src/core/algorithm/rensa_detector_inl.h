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
    complementKeyPuyosInternal(cf, cpl, strategy, 1, maxKeyPuyos, 1, callback);
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
                                               int maxPuyosAtOnce,
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
            int numPuyoAdded = 0;
            for (int i = 0; i < maxPuyosAtOnce; ++i) {
                if (!currentField.dropPuyoOn(x, c))
                    break;
                if (!currentKeyPuyos.add(x, c)) {
                    currentField.removePuyoFrom(x);
                    break;
                }

                ++numPuyoAdded;

                if (currentField.countConnectedPuyosMax4(x, currentField.height(x)) >= 4)
                    break;

                complementKeyPuyosInternal(currentField, currentKeyPuyos, strategy,
                                           x, restAdded - 1, maxPuyosAtOnce, callback);
            }

            for (int i = 0; i < numPuyoAdded; ++i) {
                currentField.removePuyoFrom(x);
                currentKeyPuyos.removeTopFrom(x);
            }
        }
    }
}

template<typename Callback>
void RensaDetector::complementKeyPuyosOn13thRow(const CoreField& originalField,
                                                const bool allowsComplements[FieldConstant::MAP_WIDTH],
                                                Callback callback)
{
    CoreField cf(originalField);
    ColumnPuyoList cpl;
    complementKeyPuyos13thRowInternal(cf, cpl, allowsComplements, 1, callback);
}

template<typename Callback>
void RensaDetector::complementKeyPuyos13thRowInternal(CoreField& currentField,
                                                      ColumnPuyoList& currentKeyPuyos,
                                                      const bool allowsComplements[FieldConstant::MAP_WIDTH],
                                                      int x,
                                                      Callback callback)
{
    if (x > 6) {
        callback(const_cast<const CoreField&>(currentField),
                 const_cast<const ColumnPuyoList&>(currentKeyPuyos));
        return;
    }

    if (currentField.height(x) == 12 && allowsComplements[x]) {
        for (PuyoColor c : NORMAL_PUYO_COLORS) {
            if (!currentField.dropPuyoOn(x, c))
                continue;
            if (!currentKeyPuyos.add(x, c)) {
                currentField.removePuyoFrom(x);
                continue;
            }

            complementKeyPuyos13thRowInternal(currentField, currentKeyPuyos, allowsComplements, x + 1, callback);

            currentField.removePuyoFrom(x);
            currentKeyPuyos.removeTopFrom(x);
        }
    }

    complementKeyPuyos13thRowInternal(currentField, currentKeyPuyos, allowsComplements, x + 1, callback);
}

template<typename Callback>
void RensaDetector::complementOneTypeKeyPuyos(const CoreField& originalField,
                                              const RensaDetectorStrategy& strategy,
                                              int maxPuyos,
                                              Callback callback)
{
    CoreField cf(originalField);
    ColumnPuyoList cpl;
    complementKeyPuyosInternal(cf, cpl, strategy, 1, 1, maxPuyos, callback);
}

// static
template<typename Callback>
void RensaDetector::detectWithAddingKeyPuyos(const CoreField& originalField,
                                             const RensaDetectorStrategy& strategy,
                                             int maxKeyPuyos,
                                             Callback callback)
{
    const bool prohibits[FieldConstant::MAP_WIDTH] {};
    auto complementCallback = [&](const CoreField& complementedField, const ColumnPuyoList& keyPuyos) {
        auto detectionCallback = [&](const CoreField& wholeComplementedField, const ColumnPuyoList& firePuyos) {
            ColumnPuyoList cpl(keyPuyos);
            if (!cpl.merge(firePuyos))
                return;

            callback(wholeComplementedField, cpl);
        };
        detect(complementedField, strategy, PurposeForFindingRensa::FOR_FIRE, prohibits, detectionCallback);
    };
    complementKeyPuyos(originalField, strategy, maxKeyPuyos, complementCallback);
}

#endif // CORE_ALGORITHM_RENSA_DETECTOR_INL_H_
