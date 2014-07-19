#ifndef CORE_ALGORITHM_RENSA_REF_SEQUENCE_H_
#define CORE_ALGORITHM_RENSA_REF_SEQUENCE_H_

#include <vector>
#include <glog/logging.h>

#include "base/base.h"
#include "core/field/rensa_result.h"

#include "core/field/core_field.h"
#include "core/algorithm/column_puyo_list.h"

class CoreField;
class ColumnPuyoList;
struct RensaResult;
class RensaTrackResult;

// TODO(mayah): make these fields reference.
struct RensaRef {
    CoreField fieldBeforeRensa;
    CoreField fieldAfterRensa;
    ColumnPuyoList keyPuyos;
    ColumnPuyoList firePuyos;
    RensaResult rensaResult;
    RensaTrackResult rensaTrackResult;
};

class RensaRefSequence : noncopyable {
public:
    void push(const RensaRef& rensa) { rensaRefs_.push_back(rensa); }
    void pop() { rensaRefs_.pop_back(); }

    // Returns the sum of chains in rensa sequence.
    int totalChains() const
    {
        int total = 0;
        for (const auto& rensa : rensaRefs_) {
            total += rensa.rensaResult.chains;
        }
        return total;
    }

    const RensaRef& combinedRensa() const
    {
        CHECK(isValidCombinedRensaRef_);
        return combinedRensaRef_;
    }

    void setCombinedRensa(const RensaRef& rensa)
    {
        DCHECK(!isValidCombinedRensaRef_) << "maybe forget to invalidate combined rensa?";
        combinedRensaRef_ = rensa;
        isValidCombinedRensaRef_ = true;
    }

    void invalidateCombinedRensa() { isValidCombinedRensaRef_ = false; }

private:
    bool isValidCombinedRensaRef_ = false;
    RensaRef combinedRensaRef_;
    std::vector<RensaRef> rensaRefs_;
};

#endif
