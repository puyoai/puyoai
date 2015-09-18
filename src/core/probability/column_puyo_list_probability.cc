#include "core/probability/column_puyo_list_probability.h"

#include <limits>

#include "core/kumipuyo.h"

using namespace std;

static const Kumipuyo ALL_KUMIPUYO_KINDS[] = {
    Kumipuyo(PuyoColor::RED, PuyoColor::RED),
    Kumipuyo(PuyoColor::RED, PuyoColor::BLUE),
    Kumipuyo(PuyoColor::RED, PuyoColor::YELLOW),
    Kumipuyo(PuyoColor::RED, PuyoColor::GREEN),
    Kumipuyo(PuyoColor::BLUE, PuyoColor::BLUE),
    Kumipuyo(PuyoColor::BLUE, PuyoColor::YELLOW),
    Kumipuyo(PuyoColor::BLUE, PuyoColor::GREEN),
    Kumipuyo(PuyoColor::YELLOW, PuyoColor::YELLOW),
    Kumipuyo(PuyoColor::YELLOW, PuyoColor::GREEN),
    Kumipuyo(PuyoColor::GREEN, PuyoColor::GREEN),
};

static double necessaryPuyosReverse(const ColumnPuyoList& cpl, unordered_map<ColumnPuyoList, double>* m)
{
    // for child state i:
    // transition possibility: p_i
    // the necessary puyos: s_i
    //
    // s = (1 + \sum p_i s_i) / \sum p_i

    auto it = m->find(cpl);
    if (it != m->end())
        return it->second;

    double s = 16;
    int puttableCount = 0;
    for (const auto& kp : ALL_KUMIPUYO_KINDS) {

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
                    p = std::min(p, necessaryPuyosReverse(tmp, m));
                }
            } else if (cpl.sizeOn(x) == 1) {
                if (cpl.top(x) == kp.axis || cpl.top(x) == kp.child) {
                    // ok
                    puttable = true;
                    used = true;

                    ColumnPuyoList tmp(cpl);
                    tmp.removeTopFrom(x);
                    p = std::min(p, necessaryPuyosReverse(tmp, m));
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
                    p = std::min(p, necessaryPuyosReverse(tmp, m));
                }
            } else if (cpl.sizeOn(x) >= 1 && cpl.sizeOn(x + 1) == 0) {
                if (cpl.top(x) == kp.axis || cpl.top(x) == kp.child) {
                    puttable = true;
                    used = true;

                    ColumnPuyoList tmp(cpl);
                    tmp.removeTopFrom(x);
                    p = std::min(p, necessaryPuyosReverse(tmp, m));
                }
            }
        }

        if (cpl.sizeOn(5) == 0 && cpl.sizeOn(6) >= 1) {
            if (cpl.top(6) == kp.axis || cpl.top(6) == kp.child) {
                puttable = true;
                used = true;

                ColumnPuyoList tmp(cpl);
                tmp.removeTopFrom(6);
                p = std::min(p, necessaryPuyosReverse(tmp, m));
            }
        }

        if (!puttable) {
            return numeric_limits<double>::infinity();
        }

        if (used) {
            if (kp.axis == kp.child) {
                s += p;
                puttableCount += 1;
            } else {
                s += p * 2;
                puttableCount += 2;
            }
        }
    }

    double p = s / puttableCount;
    (*m)[cpl] = p;
    return p;
}

static void iter(int n, int leftX, ColumnPuyoList& cpl, unordered_map<ColumnPuyoList, double>* m)
{
    necessaryPuyosReverse(cpl, m);

    for (int x = leftX; x <= 6; ++x) {
        for (PuyoColor c : NORMAL_PUYO_COLORS) {
            cpl.add(x, c);
            if (n > 0)
                iter(n - 1, x, cpl, m);
            cpl.removeTopFrom(x);
        }
    }
}

ColumnPuyoListProbability::ColumnPuyoListProbability()
{
    unordered_map<ColumnPuyoList, double> reverseMap;
    ColumnPuyoList initial;
    reverseMap[initial] = 0.0;
    iter(5, 1, initial, &reverseMap);

    //
    for (const auto& entry : reverseMap) {
        ColumnPuyoList cpl;
        for (int x = 1; x <= 6; ++x) {
            for (int i = entry.first.sizeOn(x) - 1; i >= 0; --i) {
                cpl.add(x, entry.first.get(x, i));
            }
        }

        m_[cpl] = entry.second;
    }
}

// static
const ColumnPuyoListProbability* ColumnPuyoListProbability::instanceSlow()
{
    static std::unique_ptr<ColumnPuyoListProbability> s_instance(new ColumnPuyoListProbability());
    return s_instance.get();
}

double ColumnPuyoListProbability::necessaryKumipuyos(const ColumnPuyoList& cpl) const
{
    auto it = m_.find(cpl);
    if (it == m_.end())
        return std::numeric_limits<double>::infinity();

    return it->second;
}
