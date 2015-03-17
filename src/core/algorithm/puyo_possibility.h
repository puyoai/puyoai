#ifndef CORE_ALGORITHM_PUYO_POSSIBILITY_H_
#define CORE_ALGORITHM_PUYO_POSSIBILITY_H_

#include <glog/logging.h>

#include <algorithm>

#include "core/algorithm/puyo_set.h"

class TsumoPossibility {
public:
    static const int MAX_K = 32;
    static const int MAX_N = 16;

    // Returns the possibility that when there are randomly |k| puyos,
    // that set will contain |puyoSet|.
    static double possibility(const PuyoSet& puyoSet, int k) {
        int a = std::min(MAX_N - 1, puyoSet.red());
        int b = std::min(MAX_N - 1, puyoSet.blue());
        int c = std::min(MAX_N - 1, puyoSet.yellow());
        int d = std::min(MAX_N - 1, puyoSet.green());
        int kk = std::min(MAX_K - 1, k);

        return s_possibility[a][b][c][d][kk];
    }

    // Returns how many puyos are required to get |puyoSet| with possibility |threshold|?
    static int necessaryPuyos(const PuyoSet& puyoSet, double threshold) {
        DCHECK(s_initialized) << "TsumoPossibility is not initialized.";
        DCHECK(0 <= threshold && threshold <= 1.0);

        int a = std::min(MAX_N - 1, puyoSet.red());
        int b = std::min(MAX_N - 1, puyoSet.blue());
        int c = std::min(MAX_N - 1, puyoSet.yellow());
        int d = std::min(MAX_N - 1, puyoSet.green());

        double* p = s_possibility[a][b][c][d];

        for (int k = 0; k < MAX_K; ++k) {
            if (p[k] >= threshold)
                return k;
        }

        return MAX_K;
    }

    static void initialize();

private:
    static bool s_initialized;
    static double s_possibility[MAX_N][MAX_N][MAX_N][MAX_N][MAX_K];
};

#endif
