#ifndef CPU_MAYAH_NEURAL_H_
#define CPU_MAYAH_NEURAL_H_

#include <vector>

#include "core/plain_field.h"
#include "core/kumipuyo.h"
#include "core/rensa_result.h"

struct NeuralNetRequest {
    NeuralNetRequest() = default;
    NeuralNetRequest(const PlainField& plain_field, const Kumipuyo& next1, const Kumipuyo& next2, int rest_hand) :
        plain_field(plain_field),
        next1(next1),
        next2(next2),
        rest_hand(rest_hand) {}

    PlainField plain_field;
    Kumipuyo next1;
    Kumipuyo next2;
    int rest_hand = 0;
};

struct NeuralNetResponse {
    bool ok;
    double possibility[6][4];
    double q;

    static NeuralNetResponse invalid() {
        NeuralNetResponse resp;
        resp.ok = false;
        return resp;
    }
};

std::vector<NeuralNetResponse> ask_puyo_server(const std::vector<NeuralNetRequest>& request);

#endif  // CPU_MAYAH_NEURAL_H_