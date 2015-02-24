#pragma once

#include <algorithm>

#include "core/field_constant.h"

#include "core/score.h"

namespace munetoshi {
constexpr int MAX_PUYOS = FieldConstant::WIDTH * (FieldConstant::HEIGHT + 1);
constexpr int OJAMAS_TO_KILL = FieldConstant::WIDTH * FieldConstant::HEIGHT - 3;
constexpr int SCORE_TO_KILL = OJAMAS_TO_KILL * SCORE_FOR_OJAMA;

class puyo_calc {
public:
    static inline int score_form_puyos(int puyo_num) {
        return PUYOS_TO_SCORE_TABLE[puyo_num];
    }

    static inline int puyos_from_score(int score) {
        auto itr = std::lower_bound(
                PUYOS_TO_SCORE_TABLE.begin(),
                PUYOS_TO_SCORE_TABLE.end(),
                score);
        return itr - PUYOS_TO_SCORE_TABLE.begin();
    }

private:
    puyo_calc() = delete;
    ~puyo_calc() = delete;
    puyo_calc(const puyo_calc&) = delete;
    puyo_calc(puyo_calc&&) = delete;
    puyo_calc& operator =(const puyo_calc&) = delete;
    puyo_calc& operator =(puyo_calc&&) = delete;

    const static std::array<int, MAX_PUYOS + 1> PUYOS_TO_SCORE_TABLE;

    static inline int score_for_a_chain(int puyo_num, int chains) {
        return puyo_num * 10
                * calculateRensaBonusCoef(chainBonus(chains), longBonus(puyo_num), 0);
    }

    static inline int score_for_puyos(int puyo_num) {
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

};

const std::array<int, munetoshi::MAX_PUYOS + 1> puyo_calc::PUYOS_TO_SCORE_TABLE(
        []()->std::array<int, MAX_PUYOS + 1> {
    std::array<int, MAX_PUYOS + 1> scores;
    for (int puyos = 0; puyos < MAX_PUYOS + 1; ++puyos) {
        scores[puyos] = score_for_puyos(puyos);
    }
    return scores;
}());

} // namespace munetoshi
