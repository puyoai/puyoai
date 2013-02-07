#ifndef __PUYO_POSSIBILITY_H_
#define __PUYO_POSSIBILITY_H_

#include <glog/logging.h>
#include "puyo.h"
#include "puyo_set.h"

class TsumoPossibility {
public:
    static const int MAX_K = 32;
    static const int MAX_N = 16;
    typedef double (*PossibilityArrayPtr)[MAX_N][MAX_N][MAX_N][MAX_N];

    // RED, BLUE, YELLOW, GREEN をそれぞれ少なくとも a, b, c, d 個欲しい場合に、
    // k ぷよ（組ではない）引いてそれらを得られる確率
    static double possibility(unsigned int k, unsigned int a, unsigned int b, unsigned int c, unsigned int d)
    {
        DCHECK(s_initialized) << "TsumoPossibility is not initialized.";
        DCHECK(k < MAX_K);
        DCHECK(a < MAX_N);
        DCHECK(b < MAX_N);
        DCHECK(c < MAX_N);
        DCHECK(d < MAX_N);

        return s_possibility[k][a][b][c][d];
    }

    static double possibility(unsigned int k, PuyoSet set) {
        return possibility(k, set.red(), set.blue(), set.yellow(), set.green());
    }

    static void initialize();

private:
    static bool s_initialized;
    static PossibilityArrayPtr s_possibility;
};

#endif
