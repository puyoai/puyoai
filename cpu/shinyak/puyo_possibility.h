#ifndef __PUYO_POSSIBILITY_H_
#define __PUYO_POSSIBILITY_H_

#include <glog/logging.h>
#include "puyo.h"
#include "puyo_set.h"

class TsumoPossibility {
public:
    static const int N = 32;

    // RED, BLUE, YELLOW, GREEN をそれぞれ少なくとも a, b, c, d 個欲しい場合に、
    // k ぷよ（組ではない）引いてそれらを得られる確率
    static double possibility(int k, int a, int b, int c, int d)
    {
        DCHECK(s_initialized);
        DCHECK(0 <= k && k < N);
        DCHECK(0 <= a && a < N);
        DCHECK(0 <= b && b < N);
        DCHECK(0 <= c && c < N);
        DCHECK(0 <= d && d < N);

        return s_possibility[k][a][b][c][d];
    }

    static double possibility(int k, PuyoSet set) {
        return possibility(k, set.red(), set.blue(), set.yellow(), set.green());
    }

    static void initialize();

private:
    static bool s_initialized;
    static double s_possibility[N][N][N][N][N];
};

#endif
