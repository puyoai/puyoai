#ifndef CORE_ALGORITHM_PUYO_POSSIBILITY_H_
#define CORE_ALGORITHM_PUYO_POSSIBILITY_H_

#include <glog/logging.h>

#include "core/algorithm/puyo_set.h"

class TsumoPossibility {
public:
    static const int MAX_K = 32;
    static const int MAX_N = 16;
    typedef double (*PossibilityArrayPtr)[MAX_N][MAX_N][MAX_N][MAX_N];

    // RED, BLUE, YELLOW, GREEN をそれぞれ少なくとも a, b, c, d 個欲しい場合に、
    // k ぷよ（組ではない）引いてそれらを得られる確率
    static double possibility(int k, int a, int b, int c, int d)
    {
        DCHECK(s_initialized) << "TsumoPossibility is not initialized.";
        DCHECK(0 <= k && k < MAX_K) << k;

        a = std::min(a, MAX_N - 1);
        b = std::min(b, MAX_N - 1);
        c = std::min(c, MAX_N - 1);
        d = std::min(d, MAX_N - 1);

        return s_possibility[k][a][b][c][d];
    }

    static double possibility(unsigned int k, const PuyoSet& set) {
        return possibility(k, set.red(), set.blue(), set.yellow(), set.green());
    }

    static int necessaryPuyos(double threshold, const PuyoSet& set) {
        DCHECK(0 <= threshold && threshold < 1.0);

        if (possibility(MAX_K - 1, set) < threshold)
            return MAX_K;

        DCHECK(possibility(0, set) < threshold);
        DCHECK(possibility(MAX_K - 1, set) >= threshold);

        int left = 0, right = MAX_K - 1;
        while (right - left > 1) {
            int mid = (left + right) / 2;
            if (possibility(mid, set) >= threshold) {
                right = mid;
            } else {
                left = mid;
            }
        }

        return right;
    }

    static void initialize();

private:
    static bool s_initialized;
    static PossibilityArrayPtr s_possibility;
};

#endif
