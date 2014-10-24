#ifndef CORE_ALGORITHM_PUYO_POSSIBILITY_H_
#define CORE_ALGORITHM_PUYO_POSSIBILITY_H_

#include <glog/logging.h>

#include "core/algorithm/puyo_set.h"

class TsumoPossibility {
public:
    static const int MAX_K = 32;
    static const int MAX_N = 16;

    static double possibility(const PuyoSet& puyoSet, unsigned int k) {
        int a = std::min(MAX_N - 1, puyoSet.red());
        int b = std::min(MAX_N - 1, puyoSet.blue());
        int c = std::min(MAX_N - 1, puyoSet.yellow());
        int d = std::min(MAX_N - 1, puyoSet.green());

        return possibility(a, b, c, d, k);
    }

    static int necessaryPuyos(const PuyoSet& puyoSet, double threshold) {
        DCHECK(0 <= threshold && threshold < 1.0);

        if (possibility(puyoSet, MAX_K - 1) < threshold)
            return MAX_K;

        int a = std::min(MAX_N - 1, puyoSet.red());
        int b = std::min(MAX_N - 1, puyoSet.blue());
        int c = std::min(MAX_N - 1, puyoSet.yellow());
        int d = std::min(MAX_N - 1, puyoSet.green());

        DCHECK(possibility(a, b, c, d, 0) < threshold);
        DCHECK(possibility(a, b, c, d, MAX_K - 1) >= threshold);

        int left = 0, right = MAX_K - 1;
        while (right - left > 1) {
            int mid = (left + right) / 2;
            if (possibility(a, b, c, d, mid) >= threshold) {
                right = mid;
            } else {
                left = mid;
            }
        }

        return right;
    }

    static void initialize();

private:
    // RED, BLUE, YELLOW, GREEN をそれぞれ少なくとも a, b, c, d 個欲しい場合に、
    // k ぷよ（組ではない）引いてそれらを得られる確率
    static double possibility(int a, int b, int c, int d, int k)
    {
        DCHECK(s_initialized) << "TsumoPossibility is not initialized.";
        DCHECK(0 <= a && a < MAX_N) << a;
        DCHECK(0 <= b && b < MAX_N) << b;
        DCHECK(0 <= c && c < MAX_N) << c;
        DCHECK(0 <= d && d < MAX_N) << d;
        DCHECK(0 <= k && k < MAX_K) << k;

        return s_possibility[a][b][c][d][k];
    }

    static bool s_initialized;
    static double s_possibility[MAX_N][MAX_N][MAX_N][MAX_N][MAX_K];
};

#endif
