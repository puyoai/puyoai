#ifndef CORE_ALGORITHM_RENSA_REF_SEQUENCE_H_
#define CORE_ALGORITHM_RENSA_REF_SEQUENCE_H_

#include <vector>

#include "base/noncopyable.h"
#include "core/rensa_result.h"

class CoreField;
class ColumnPuyoList;

struct RensaRef {
    const CoreField& fieldBeforeRensa;
    const CoreField& fieldAfterRensa;
    const ColumnPuyoList& keyPuyos;
    const ColumnPuyoList& firePuyos;
    const RensaResult& rensaResult;
    const RensaTrackResult& rensaTrackResult;
};

class RensaRefSequence : noncopyable {
public:
    void push(const RensaRef* rensa) { rensaRefs_.push_back(rensa); }
    void pop() { rensaRefs_.pop_back(); }

    // Returns the sum of chains in rensa sequence.
    int totalChains() const
    {
        int total = 0;
        for (const auto& rensa : rensaRefs_)
            total += rensa->rensaResult.chains;
        return total;
    }

private:
    std::vector<const RensaRef*> rensaRefs_;
};

#endif
