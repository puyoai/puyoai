#include "core/rensa_tracker/rensa_coef_tracker.h"

int RensaCoefResult::score(int additionalChain) const
{
    int rensaCoef[20] {};
    int rensaNumErased[20] {};
    for (int i = 1; i <= additionalChain; ++i) {
        rensaCoef[i] = chainBonus(i);
        if (rensaCoef[i] == 0)
            rensaCoef[i] = 1;
        rensaNumErased[i] = 4;
    }

    for (int i = additionalChain + 1, j = 1; i <= 19 && numErased(j) > 0; ++i, ++j) {
        rensaCoef[i] = calculateRensaBonusCoef(chainBonus(i), longBonusCoef(j), colorBonusCoef(j));
        rensaNumErased[i] = numErased(j);
    }

    int sum = 0;
    for (int i = 1; i <= 19 && rensaNumErased[i] > 0; ++i) {
        sum += rensaCoef[i] * rensaNumErased[i] * 10;
    }

    return sum;
}
