#ifndef CORE_PROBABILITY_PUYO_POSSIBILITY_H_
#define CORE_PROBABILITY_PUYO_POSSIBILITY_H_

#include <glog/logging.h>

#include <algorithm>
#include <unordered_map>

#include "base/noncopyable.h"
#include "core/column_puyo_list.h"
#include "core/probability/puyo_set.h"

class KumipuyoSeq;

class PuyoSetProbability : noncopyable, nonmovable {
public:
    // Returns PuyoSetProbability instance. This might take time.
    static const PuyoSetProbability* instanceSlow();

    PuyoSetProbability();

    // Returns the possibility that when there are randomly |k| puyos,
    // that set will contain |puyoSet|.
    double possibility(const PuyoSet& puyoSet, int k) const
    {
        int a = std::min(MAX_N - 1, puyoSet.red());
        int b = std::min(MAX_N - 1, puyoSet.blue());
        int c = std::min(MAX_N - 1, puyoSet.yellow());
        int d = std::min(MAX_N - 1, puyoSet.green());
        int kk = std::min(MAX_K - 1, k);

        return p_[a][b][c][d][kk];
    }

    // Returns how many puyos are required to get |puyoSet| with possibility |threshold|?
    int necessaryPuyos(const PuyoSet& puyoSet, double threshold) const
    {
        DCHECK(0 <= threshold && threshold <= 1.0) << threshold;

        int a = std::min(MAX_N - 1, puyoSet.red());
        int b = std::min(MAX_N - 1, puyoSet.blue());
        int c = std::min(MAX_N - 1, puyoSet.yellow());
        int d = std::min(MAX_N - 1, puyoSet.green());

        const double* p = p_[a][b][c][d];

        for (int k = 0; k < MAX_K; ++k) {
            if (p[k] >= threshold)
                return k;
        }

        return MAX_K;
    }

    // Returns the number of puyos to get |PuyoSet| with possibility |threshold|.
    // Some of kumipuyo seq is provided.
    int necessaryPuyos(const PuyoSet&, const KumipuyoSeq&, double threshold) const;

private:
    static const int MAX_N = 16;
    static const int MAX_K = 32;

    double p_[MAX_N][MAX_N][MAX_N][MAX_N][MAX_K];
};

#endif // CORE_PROBABILITY_PUYO_POSSIBILITY_H_
