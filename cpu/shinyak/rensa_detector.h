#ifndef __RENSA_DETECTOR_H_
#define __RENSA_DETECTOR_H_

#include <vector>

#include "puyo_set.h"
#include "rensa_info.h"

class Field;

class RensaDetector {
public:
    // 現在のフィールドから発火可能な連鎖を列挙する。
    static void findRensas(std::vector<PossibleRensaInfo>& result, const Field&, const PuyoSet& additionalPuyoSet = PuyoSet());

    // Before calling findRensa, adding some puyos (numMaxAddedPuyo) is allowed. 
    static void findPossibleRensas(std::vector<PossibleRensaInfo>& result, const Field&, int numMaxAddedPuyo);

    // 与えられた組ぷよだけを使って発火可能な連鎖を求める
    static void findFeasibleRensas(std::vector<FeasibleRensaInfo>& result, const Field&, int numKumiPuyo, const std::vector<KumiPuyo>& kumiPuyos);

    // 連鎖を再帰的に打つことで可能な連鎖を求める。不可能な連鎖も列挙されてしまうはず。
    // TODO: 不可能な連鎖を列挙させない方法を考える。
    static void findPossibleRensasUsingIteration(std::vector<PossibleRensaInfo>& result, const Field&, int maxIteration, int additionalChain = 0, PuyoSet additionalPuyoSet = PuyoSet());
};

#endif 
