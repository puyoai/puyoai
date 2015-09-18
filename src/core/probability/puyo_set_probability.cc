#include "core/probability/puyo_set_probability.h"

#include <algorithm>

#include "core/kumipuyo_seq.h"

using namespace std;

bool PuyoSetProbability::s_initialized = false;
double PuyoSetProbability::s_possibility[MAX_N][MAX_N][MAX_N][MAX_N][MAX_K];

std::unordered_map<ColumnPuyoList, double> PuyoSetProbability::s_m;

// static
void PuyoSetProbability::initialize()
{
    if (s_initialized)
        return;

    s_initialized = true;
    initializePuyoSetProbability();
    initializeColumnPuyoListProbability();
}

// static
void PuyoSetProbability::initializePuyoSetProbability()
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
                        s_possibility[a][b][c][d][k] = p[a][b][c][d][k];
                    }
                }
            }
        }
    }

    delete[] p;
    delete[] q;
}

// static
void PuyoSetProbability::initializeColumnPuyoListProbability()
{
    s_m[ColumnPuyoList()] = 0;

    std::function<void (ColumnPuyoList& cpl, int, int)> f;
    f = [&](ColumnPuyoList& cpl, int leftX, int rest) {
        if (rest == 0)
            return;

        for (int x = leftX; x <= 6; ++x) {
            for (PuyoColor c : NORMAL_PUYO_COLORS) {
                cpl.add(x, c);
                necessaryPuyos(cpl);
                f(cpl, x, rest - 1);
                cpl.removeTopFrom(x);
            }
        }
    };

    // TODO(mayah): slow. We'd like to have rest = 6 for cache, but it takes about 7 seconds
    // to calculate now.
    ColumnPuyoList cpl;
    f(cpl, 1, 3);
}

// static
int PuyoSetProbability::necessaryPuyos(const PuyoSet& puyoSet, const KumipuyoSeq& seq, double threshold)
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

// static
double PuyoSetProbability::necessaryPuyos(const ColumnPuyoList& cpl)
{
    ColumnPuyoList cplReverse;
    for (int x = 1; x <= 6; ++x) {
        for (int j = cpl.sizeOn(x) - 1; j >= 0; --j) {
            cplReverse.add(x, cpl.get(x, j));
        }
    }

    return necessaryPuyosReverse(cplReverse);
}

// static
double PuyoSetProbability::necessaryPuyosReverse(const ColumnPuyoList& cpl)
{
    // for child state i:
    // transition possibility: p_i
    // the necessary puyos: s_i
    //
    // s = (1 + \sum p_i s_i) / \sum p_i

    auto it = s_m.find(cpl);
    if (it != s_m.end())
        return it->second;

    static const Kumipuyo kumipuyos[] = {
        Kumipuyo(PuyoColor::RED, PuyoColor::RED),
        Kumipuyo(PuyoColor::RED, PuyoColor::BLUE),
        Kumipuyo(PuyoColor::RED, PuyoColor::YELLOW),
        Kumipuyo(PuyoColor::RED, PuyoColor::GREEN),
        Kumipuyo(PuyoColor::BLUE, PuyoColor::RED),
        Kumipuyo(PuyoColor::BLUE, PuyoColor::BLUE),
        Kumipuyo(PuyoColor::BLUE, PuyoColor::YELLOW),
        Kumipuyo(PuyoColor::BLUE, PuyoColor::GREEN),
        Kumipuyo(PuyoColor::YELLOW, PuyoColor::RED),
        Kumipuyo(PuyoColor::YELLOW, PuyoColor::BLUE),
        Kumipuyo(PuyoColor::YELLOW, PuyoColor::YELLOW),
        Kumipuyo(PuyoColor::YELLOW, PuyoColor::GREEN),
        Kumipuyo(PuyoColor::GREEN, PuyoColor::RED),
        Kumipuyo(PuyoColor::GREEN, PuyoColor::BLUE),
        Kumipuyo(PuyoColor::GREEN, PuyoColor::YELLOW),
        Kumipuyo(PuyoColor::GREEN, PuyoColor::GREEN),
    };

    double s = 16;
    int puttableCount = 0;
    for (const auto& kp : kumipuyos) {

        double p = numeric_limits<double>::infinity();
        bool puttable = false;
        bool used = false;

        // put vertically
        for (int x = 1; x <= 6; ++x) {
            if (cpl.sizeOn(x) >= 2) {
                if ((cpl.get(x, cpl.sizeOn(x) - 1) == kp.axis && cpl.get(x, cpl.sizeOn(x) - 2) == kp.child) ||
                    (cpl.get(x, cpl.sizeOn(x) - 1) == kp.child && cpl.get(x, cpl.sizeOn(x) - 2) == kp.axis)) {
                    // ok
                    puttable = true;
                    used = true;

                    ColumnPuyoList tmp(cpl);
                    tmp.removeTopFrom(x);
                    tmp.removeTopFrom(x);
                    p = std::min(p, necessaryPuyosReverse(tmp));
                }
            } else if (cpl.sizeOn(x) == 1) {
                if (cpl.top(x) == kp.axis || cpl.top(x) == kp.child) {
                    // ok
                    puttable = true;
                    used = true;

                    ColumnPuyoList tmp(cpl);
                    tmp.removeTopFrom(x);
                    p = std::min(p, necessaryPuyosReverse(tmp));
                }
            } else {
                // ok, but not used.
                puttable = true;
            }
        }

        // put horizontally
        for (int x = 1; x <= 5; ++x) {
            if (cpl.sizeOn(x) >= 1 && cpl.sizeOn(x + 1) >= 1) {
                if ((cpl.top(x) == kp.axis && cpl.top(x + 1) == kp.child) || (cpl.top(x) == kp.child && cpl.top(x + 1) == kp.axis)) {
                    puttable = true;
                    used = true;

                    ColumnPuyoList tmp(cpl);
                    tmp.removeTopFrom(x);
                    tmp.removeTopFrom(x + 1);
                    p = std::min(p, necessaryPuyosReverse(tmp));
                }
            } else if (cpl.sizeOn(x) >= 1 && cpl.sizeOn(x + 1) == 0) {
                if (cpl.top(x) == kp.axis || cpl.top(x) == kp.child) {
                    puttable = true;
                    used = true;

                    ColumnPuyoList tmp(cpl);
                    tmp.removeTopFrom(x);
                    p = std::min(p, necessaryPuyosReverse(tmp));
                }
            }
        }

        if (cpl.sizeOn(5) == 0 && cpl.sizeOn(6) >= 1) {
            if (cpl.top(6) == kp.axis || cpl.top(6) == kp.child) {
                puttable = true;
                used = true;

                ColumnPuyoList tmp(cpl);
                tmp.removeTopFrom(6);
                p = std::min(p, necessaryPuyosReverse(tmp));
            }
        }

        if (!puttable) {
            return numeric_limits<double>::infinity();
        }

        if (used) {
            s += p;
            puttableCount += 1;
        }
    }

    double p = s / puttableCount;
    s_m[cpl] = p;
    return p;
}
