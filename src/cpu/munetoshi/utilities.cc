#include "utilities.h"

namespace {
inline int score_for_a_chain(int puyo_num, int chains) {
    return puyo_num * 10
            * calculateRensaBonusCoef(chainBonus(chains), longBonus(puyo_num), 0);
}

inline int score_for_puyos(int puyo_num) {
    if (puyo_num < 4) {
        return 0;
    }
    int score = 0;
    int chains = 1;
    for (; puyo_num >= 8; puyo_num -= 4, chains += 1) {
        score += score_for_a_chain(4, chains);
    }
    score += score_for_a_chain(puyo_num, chains);
    return score;
}

} // namespace

const std::array<int, munetoshi::MAX_PUYOS + 1> munetoshi::puyo_calc::PUYOS_TO_SCORE_TABLE =
        []()->std::array<int, MAX_PUYOS + 1> {
    std::array<int, MAX_PUYOS + 1> scores;
    for (int puyos = 0; puyos < MAX_PUYOS + 1; ++puyos) {
        scores[puyos] = score_for_puyos(puyos);
    }
    return scores;
}();




