#include "puyo_possibility.h"

#include <algorithm>

using namespace std;

bool TsumoPossibility::s_initialized = false;
double TsumoPossibility::s_possibility[TsumoPossibility::N][TsumoPossibility::N][TsumoPossibility::N][TsumoPossibility::N][TsumoPossibility::N];

struct TsumoPossibilityCalc {
    static const int N = TsumoPossibility::N;

    TsumoPossibilityCalc() {
        for (int a = 0; a < N; ++a) {
            for (int b = 0; b < N; ++b) {
                for (int c = 0; c < N; ++c) {
                    for (int d = 0; d < N; ++d) {
                        p[a][b][c][d] = 0;
                    }
                }
            }
        }

        p[0][0][0][0] = 1;

        for (int a = 0; a < N; ++a) {
            for (int b = 0; b < N; ++b) {
                for (int c = 0; c < N; ++c) {
                    for (int d = 0; d < N; ++d) {
                        if (a + 1 < N)
                            p[a+1][b][c][d] += p[a][b][c][d] / 4;
                        if (b + 1 < N)
                            p[a][b+1][c][d] += p[a][b][c][d] / 4;
                        if (c + 1 < N)
                            p[a][b][c+1][d] += p[a][b][c][d] / 4;
                        if (d + 1 < N)
                            p[a][b][c][d+1] += p[a][b][c][d] / 4;
                    }
                }
            }
        }
        
        for (int t = 0; t < 5; ++t) {
            for (int k = 0; k < N; ++k) {
                for (int a = 0; a < N; ++a) {
                    for (int b = 0; b < N; ++b) {
                        for (int c = 0; c < N; ++c) {
                            for (int d = 0; d < N; ++d) {
                                q[t][k][a][b][c][d] = -1;
                            }
                        }
                    }
                }
            }
        }
    }

    void set(int t, int k, int a, int b, int c, int d)
    {
        int idx[] = { 0, 1, 2, 3 };
        int val[] = { a, b, c, d };
        do {
            int i0 = val[idx[0]];
            int i1 = val[idx[1]];
            int i2 = val[idx[2]];
            int i3 = val[idx[3]];

            q[t][k][i0][i1][i2][i3] = p[a][b][c][d];
        } while (next_permutation(idx, idx + 4));
    }

    // k 個使って、a, b, c, d のうち最初の t 個は strictly equal で、残りが equal or more
    // を許すような場合の確率
    // (a↑, b↑, c↑, d↑) = (a+1↑, b↑, c↑, d↑) + (a, b↑, c↑, d↑)
    double calc(int t, int k, int a, int b, int c, int d)
    {
        int s = a + b + c + d;
        if (k < s)
            return 0;

        if (q[t][k][a][b][c][d] >= 0)
            return q[t][k][a][b][c][d];

        if (t == 0) {
            if (k == s) {
                set(t, k, a, b, c, d);
                return p[a][b][c][d];
            } else
                return q[t][k][a][b][c][d] = 0;
        }

        if (t == 1)
            return q[t][k][a][b][c][d] = calc(0, k, a, b, c, d) + calc(1, k, a, b, c, d + 1);

        if (t == 2)
            return q[t][k][a][b][c][d] = calc(1, k, a, b, c, d) + calc(2, k, a, b, c + 1, d);

        if (t == 3)
            return q[t][k][a][b][c][d] = calc(2, k, a, b, c, d) + calc(3, k, a, b + 1, c, d);

        if (t == 4)
            return q[t][k][a][b][c][d] = calc(3, k, a, b, c, d) + calc(4, k, a + 1, b, c, d);

        DCHECK(false);
        return -1;
    }

    double p[N][N][N][N];
    double q[5][N][N][N][N][N];
};

void TsumoPossibility::initialize()
{
    if (s_initialized)
        return;

    TsumoPossibilityCalc* p = new TsumoPossibilityCalc();

    for (int k = 0; k < N; ++k) {
        for (int a = 0; a < N; ++a) {
            for (int b = 0; b < N; ++b) {
                for (int c = 0; c < N; ++c) {
                    for (int d = 0; d < N; ++d) {
                        s_possibility[k][a][b][c][d] = p->calc(4, k, a, b, c, d);
                    }
                }
            }
        }
    }

    delete p;
    s_initialized = true;
}
