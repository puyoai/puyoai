#pragma once

#include <array>
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
        return static_cast<int>(itr - PUYOS_TO_SCORE_TABLE.begin());
    }

private:
    puyo_calc() = delete;
    ~puyo_calc() = delete;
    puyo_calc(const puyo_calc&) = delete;
    puyo_calc(puyo_calc&&) = delete;
    puyo_calc& operator =(const puyo_calc&) = delete;
    puyo_calc& operator =(puyo_calc&&) = delete;

    const static std::array<int, MAX_PUYOS + 1> PUYOS_TO_SCORE_TABLE;
};

} // namespace munetoshi
