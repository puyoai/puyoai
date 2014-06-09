#include "core/algorithm/puyo_possibility.h"

#include <algorithm>

using namespace std;

bool TsumoPossibility::s_initialized = false;
TsumoPossibility::PossibilityArrayPtr TsumoPossibility::s_possibility;

const int MAX_K = TsumoPossibility::MAX_K;
const int MAX_N = TsumoPossibility::MAX_N;

static void zeroClear(TsumoPossibility::PossibilityArrayPtr p)
{
    for (int k = 0; k < MAX_K; ++k) {
        for (int a = 0; a < MAX_N; ++a) {
            for (int b = 0; b < MAX_N; ++b) {
                for (int c = 0; c < MAX_N; ++c) {
                    for (int d = 0; d < MAX_N; ++d)
                        p[k][a][b][c][d] = 0;
                }
            }
        }
    }
}

static TsumoPossibility::PossibilityArrayPtr make() {
    TsumoPossibility::PossibilityArrayPtr p = new double[MAX_K][MAX_N][MAX_N][MAX_N][MAX_N];
    TsumoPossibility::PossibilityArrayPtr q = new double[MAX_K][MAX_N][MAX_N][MAX_N][MAX_N];

    zeroClear(p);

    p[0][0][0][0][0] = 1;
    for (int a = 0; a + 1 < MAX_N; ++a) {
        for (int b = 0; b + 1 < MAX_N && a + b + 1 < MAX_K; ++b) {
            for (int c = 0; c + 1 < MAX_N && a + b + c + 1 < MAX_K; ++c) {
                for (int d = 0; b + 1 < MAX_N && a + b + c + d + 1 < MAX_K; ++d) {
                    int k = a + b + c + d;
                    p[k+1][a+1][b][c][d] += p[k][a][b][c][d] / 4;
                    p[k+1][a][b+1][c][d] += p[k][a][b][c][d] / 4;
                    p[k+1][a][b][c+1][d] += p[k][a][b][c][d] / 4;
                    p[k+1][a][b][c][d+1] += p[k][a][b][c][d] / 4;
                }
            }
        }
    }

    for (int t = 1; t <= 4; ++t) {
        swap(p, q);
        zeroClear(p);
        for (int k = 0; k < MAX_K; ++k) {
            for (int a = MAX_N - 1; a >= 0; --a) {
                for (int b = MAX_N - 1; b >= 0; --b) {
                    for (int c = MAX_N - 1; c >= 0; --c) {
                        for (int d = MAX_N - 1; d >= 0; --d) {
                            switch (t) {
                            case 1:
                                if (d + 1 < MAX_N)
                                    p[k][a][b][c][d] = p[k][a][b][c][d+1] + q[k][a][b][c][d];
                                else
                                    p[k][a][b][c][d] = q[k][a][b][c][d];
                                break;
                            case 2:
                                if (c + 1 < MAX_N)
                                    p[k][a][b][c][d] = p[k][a][b][c+1][d] + q[k][a][b][c][d];
                                else
                                    p[k][a][b][c][d] = q[k][a][b][c][d];
                                break;
                            case 3:
                                if (b + 1 < MAX_N)
                                    p[k][a][b][c][d] = p[k][a][b+1][c][d] + q[k][a][b][c][d];
                                else
                                    p[k][a][b][c][d] = q[k][a][b][c][d];
                                break;
                            case 4:
                                if (a + 1 < MAX_N)
                                    p[k][a][b][c][d] = p[k][a+1][b][c][d] + q[k][a][b][c][d];
                                else
                                    p[k][a][b][c][d] = q[k][a][b][c][d];
                                break;
                            default:
                                DCHECK(false);
                            }
                        }
                    }
                }
            }
        }
    }

    delete[] q;
    return p;
}

void TsumoPossibility::initialize()
{
    if (s_initialized)
        return;

    s_possibility = make();
    s_initialized = true;
}
