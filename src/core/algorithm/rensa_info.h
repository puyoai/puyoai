#ifndef CORE_ALGORITHM_RENSA_INFO_H_
#define CORE_ALGORITHM_RENSA_INFO_H_

#include <string>
#include "core/algorithm/column_puyo_list.h"
#include "core/rensa_result.h"

// TODO(mayah): Consider move this to somewhere.
class FeasibleRensaInfo {
public:
    FeasibleRensaInfo() {}
    FeasibleRensaInfo(const RensaResult& rensaResult, int framesToInitiate) :
        rensaResult_(rensaResult), framesToInitiate_(framesToInitiate)
    {
    }

    const RensaResult& rensaResult() const { return rensaResult_; }

    int score() const { return rensaResult_.score; }
    int chains() const { return rensaResult_.chains; }
    int totalFrames() const { return rensaResult_.frames + framesToInitiate_; }
    int framesToInitiate() const { return framesToInitiate_; }

private:
    RensaResult rensaResult_;
    int framesToInitiate_;
};

#endif
