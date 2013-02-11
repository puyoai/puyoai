#ifndef __RENSA_DETECTOR_H_
#define __RENSA_DETECTOR_H_

#include <vector>

#include "puyo_set.h"
#include "rensa_info.h"

class Field;

class RensaDetector {
public:
    // Finds possible rensas from the specified field.
    // We might add extra puyos before finding rensas.
    static void findPossibleRensas(std::vector<PossibleRensaInfo>& result,
                                   const Field&, 
                                   int numExtraAddedPuyos = 0,
                                   const PuyoSet& additionalPuyoSet = PuyoSet());
    static void findPossibleRensas(std::vector<TrackedPossibleRensaInfo>& result,
                                   const Field&,
                                   int numExtraAddedPuyos = 0,
                                   const PuyoSet& additionalPuyoSet = PuyoSet());

    // 連鎖を再帰的に打つことで可能な連鎖を求める。不可能な連鎖も列挙されてしまうはず。
    // TODO: 不可能な連鎖を列挙させない方法を考える。
    // We don't have a tracked-version for this method.
    static void findPossibleRensasUsingIteration(std::vector<PossibleRensaInfo>& result,
                                                 const Field&,
                                                 int maxIteration,
                                                 int additionalChain = 0,
                                                 PuyoSet additionalPuyoSet = PuyoSet());

    // Finds feasible rensas.
    // Feasible rensa means a rensa which can be fired using the specified Kumipuyos.
    // We don't have a tracked-version for this method.
    static void findFeasibleRensas(std::vector<FeasibleRensaInfo>& result,
                                   const Field&,
                                   int numKumiPuyo,
                                   const std::vector<KumiPuyo>& kumiPuyos);
};

#endif 
