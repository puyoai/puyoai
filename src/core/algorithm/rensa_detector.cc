#include "rensa_detector.h"

#include <iostream>
#include <set>
#include <vector>

#include "core/algorithm/column_puyo_list.h"
#include "core/algorithm/plan.h"
#include "core/algorithm/rensa_info.h"
#include "core/decision.h"
#include "core/field/core_field.h"
#include "core/kumipuyo.h"

using namespace std;

std::vector<FeasibleRensaInfo>
RensaDetector::findFeasibleRensas(const CoreField& field, const KumipuyoSeq& kumipuyoSeq)
{
    std::vector<FeasibleRensaInfo> result;
    Plan::iterateAvailablePlans(field, kumipuyoSeq, kumipuyoSeq.size(), [&result](const RefPlan& plan) {
        if (plan.isRensaPlan())
            result.emplace_back(plan.rensaResult(), plan.initiatingFrames());
    });

    return result;
}

static inline void simulateInternal(CoreField* f, PossibleRensaInfo* info, int initialChain)
{
    info->rensaInfo = f->simulate(initialChain);
}

static inline void simulateInternal(CoreField* f, TrackedPossibleRensaInfo* info, int initialChain)
{
    info->rensaInfo = f->simulateAndTrack(&info->trackResult, initialChain);
}

template<typename T>
static void findPossibleRensasInternal(const CoreField& field,
                                       const ColumnPuyoList& addedPuyos,
                                       int leftX,
                                       int restAdded,
                                       std::vector<T>* result)
{
    RensaDetector::findRensas(field, [result, addedPuyos](CoreField* f, int x, PuyoColor c, int n) {
            T info;
            simulateInternal(f, &info, 1);
            ColumnPuyoList puyos(addedPuyos);
            for (int i = 0; i < n; ++i)
                puyos.addPuyo(x, c);
            info.necessaryPuyoSet = puyos;
            result->push_back(info);
    });

    if (restAdded <= 0)
        return;

    CoreField f(field);
    ColumnPuyoList puyoList(addedPuyos);

    for (int x = leftX; x <= CoreField::WIDTH; ++x) {
        if (f.height(x) >= 13)
            continue;

        for (int i = 0; i < NUM_NORMAL_PUYO_COLORS; ++i) {
            PuyoColor c = normalPuyoColorOf(i);

            f.dropPuyoOn(x, c);
            puyoList.addPuyo(x, c);

            if (f.connectedPuyoNums(x, f.height(x)) < 4)
                findPossibleRensasInternal(f, puyoList, x, restAdded - 1, result);

            f.removeTopPuyoFrom(x);
            puyoList.removeLastAddedPuyo();
        }
    }
}

std::vector<PossibleRensaInfo>
RensaDetector::findPossibleRensas(const CoreField& field, int maxKeyPuyos)
{
    std::vector<PossibleRensaInfo> result;
    result.reserve(100000);

    ColumnPuyoList puyoList;
    findPossibleRensasInternal(field, puyoList, 1, maxKeyPuyos, &result);
    return result;
}

std::vector<TrackedPossibleRensaInfo>
RensaDetector::findPossibleRensasWithTracking(const CoreField& field, int maxKeyPuyos)
{
    std::vector<TrackedPossibleRensaInfo> result;
    result.reserve(100000);

    ColumnPuyoList puyoList;
    findPossibleRensasInternal(field, puyoList, 1, maxKeyPuyos, &result);
    return result;
}
