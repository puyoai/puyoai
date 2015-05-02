#ifndef CORE_SCORE_H_
#define CORE_SCORE_H_

#include <glog/logging.h>

#include "core/constant.h"

constexpr int scoreForOjama(int num)
{
    return num * SCORE_FOR_OJAMA;
}

inline int chainBonus(int nthChain)
{
    static const int CHAIN_BONUS[] = {
        0,   0,   8,  16,  32,  64,  96, 128, 160, 192,
      224, 256, 288, 320, 352, 384, 416, 448, 480, 512,
    };

    DCHECK(0 <= nthChain && nthChain <= 19) << nthChain;
    return CHAIN_BONUS[nthChain];
}

inline int colorBonus(int numColors)
{
    static const int COLOR_BONUS[] = {
        0, 0, 3, 6, 12, 24,
    };

    DCHECK(0 <= numColors && numColors <= 5) << numColors;
    return COLOR_BONUS[numColors];
}

inline int longBonus(int numPuyos)
{
    static const int LONG_BONUS[] = {
        0, 0, 0, 0, 0, 2, 3, 4, 5, 6, 7, 10,
    };

    DCHECK(0 <= numPuyos) << numPuyos;
    if (numPuyos > 11)
        numPuyos = 11;

    return LONG_BONUS[numPuyos];
}

inline int calculateRensaBonusCoef(int chainBonusCoef, int longBonusCoef, int colorBonusCoef)
{
    int coef = chainBonusCoef + longBonusCoef + colorBonusCoef;
    if (coef == 0)
        return 1;
    if (coef > 999)
        return 999;
    return coef;
}

#endif  // CORE_SCORE_H_
