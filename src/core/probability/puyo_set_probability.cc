#include "core/probability/puyo_set_probability.h"

#include <algorithm>
#include <memory>

#include "core/kumipuyo_seq.h"

using namespace std;

PuyoSetProbability::PuyoSetProbability()
{
    auto p = new double[MAX_N][MAX_N][MAX_N][MAX_N][MAX_K];
    auto q = new double[MAX_N][MAX_N][MAX_N][MAX_N][MAX_K];

    for (int a = 0; a < MAX_N; ++a) {
        for (int b = 0; b < MAX_N; ++b) {
            for (int c = 0; c < MAX_N; ++c) {
                for (int d = 0; d < MAX_N; ++d) {
                    for (int k = 0; k < MAX_K; ++k) {
                        p[a][b][c][d][k] = 0.0;
                        q[a][b][c][d][k] = 0.0;
                    }
                }
            }
        }
    }

    p[0][0][0][0][0] = 1;
    for (int a = 0; a + 1 < MAX_N; ++a) {
        for (int b = 0; b + 1 < MAX_N && a + b + 1 < MAX_K; ++b) {
            for (int c = 0; c + 1 < MAX_N && a + b + c + 1 < MAX_K; ++c) {
                for (int d = 0; d + 1 < MAX_N && a + b + c + d + 1 < MAX_K; ++d) {
                    int k = a + b + c + d;
                    p[a+1][b][c][d][k+1] += p[a][b][c][d][k] / 4;
                    p[a][b+1][c][d][k+1] += p[a][b][c][d][k] / 4;
                    p[a][b][c+1][d][k+1] += p[a][b][c][d][k] / 4;
                    p[a][b][c][d+1][k+1] += p[a][b][c][d][k] / 4;
                }
            }
        }
    }

    for (int t = 1; t <= 4; ++t) {
        swap(p, q);

        // Clear p.
        for (int a = 0; a < MAX_N; ++a) {
            for (int b = 0; b < MAX_N; ++b) {
                for (int c = 0; c < MAX_N; ++c) {
                    for (int d = 0; d < MAX_N; ++d) {
                        for (int k = 0; k < MAX_K; ++k) {
                            p[a][b][c][d][k] = 0.0;
                        }
                    }
                }
            }
        }

        for (int a = MAX_N - 1; a >= 0; --a) {
            for (int b = MAX_N - 1; b >= 0; --b) {
                for (int c = MAX_N - 1; c >= 0; --c) {
                    for (int d = MAX_N - 1; d >= 0; --d) {
                        for (int k = 0; k < MAX_K; ++k) {
                            switch (t) {
                            case 1:
                                if (d + 1 < MAX_N)
                                    p[a][b][c][d][k] = p[a][b][c][d+1][k] + q[a][b][c][d][k];
                                else
                                    p[a][b][c][d][k] = q[a][b][c][d][k];
                                break;
                            case 2:
                                if (c + 1 < MAX_N)
                                    p[a][b][c][d][k] = p[a][b][c+1][d][k] + q[a][b][c][d][k];
                                else
                                    p[a][b][c][d][k] = q[a][b][c][d][k];
                                break;
                            case 3:
                                if (b + 1 < MAX_N)
                                    p[a][b][c][d][k] = p[a][b+1][c][d][k] + q[a][b][c][d][k];
                                else
                                    p[a][b][c][d][k] = q[a][b][c][d][k];
                                break;
                            case 4:
                                if (a + 1 < MAX_N)
                                    p[a][b][c][d][k] = p[a+1][b][c][d][k] + q[a][b][c][d][k];
                                else
                                    p[a][b][c][d][k] = q[a][b][c][d][k];
                                break;
                            default:
                                CHECK(false) << t;
                            }
                        }
                    }
                }
            }
        }
    }

    for (int a = 0; a < MAX_N; ++a) {
        for (int b = 0; b < MAX_N; ++b) {
            for (int c = 0; c < MAX_N; ++c) {
                for (int d = 0; d < MAX_N; ++d) {
                    for (int k = 0; k < MAX_K; ++k) {
                        p_[a][b][c][d][k] = p[a][b][c][d][k];
                    }
                }
            }
        }
    }

    delete[] p;
    delete[] q;
}

const PuyoSetProbability* PuyoSetProbability::instanceSlow()
{
    static std::unique_ptr<PuyoSetProbability> s_instance(new PuyoSetProbability);
    return s_instance.get();
}

int PuyoSetProbability::necessaryPuyos(const PuyoSet& puyoSet, const KumipuyoSeq& seq, double threshold) const
{
    PuyoSet ps(puyoSet);

    for (int i = 0; i < seq.size(); ++i) {
        if (ps.isEmpty())
            return i * 2;
        ps.sub(seq.axis(i));
        ps.sub(seq.child(i));
    }

    return seq.size() * 2 + necessaryPuyos(ps, threshold);
}
