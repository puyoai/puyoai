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
        DCHECK(0 <= k && k < MAX_K);
        DCHECK(0 <= a && a < MAX_N);
        DCHECK(0 <= b && b < MAX_N);
        DCHECK(0 <= c && c < MAX_N);
        DCHECK(0 <= d && d < MAX_N);

        return s_possibility[k][a][b][c][d];
    }

    static double possibility(unsigned int k, const PuyoSet& set) {
        return possibility(k, set.red(), set.blue(), set.yellow(), set.green());
    }

    static int necessaryPuyos(double threshold, const PuyoSet& set) {
        DCHECK(0 <= threshold && threshold < 1.0);
        for (unsigned int k = 0; k < MAX_K; ++k) {
            if (possibility(k, set) >= threshold)
                return k;
        }

        return MAX_K;
    }

    static void initialize();

private:
    static bool s_initialized;
    static PossibilityArrayPtr s_possibility;
};

#endif
