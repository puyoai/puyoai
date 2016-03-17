#include "solver/puyop.h"

#include <sstream>

#include "core/decision.h"
#include "core/kumipuyo_seq.h"

using namespace std;

string makePuyopURL(const KumipuyoSeq& seq, const vector<Decision>& decisions)
{
    static const char ENCODER[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]";
    stringstream ss;

    ss << "http://www.puyop.com/s/_";

    for (size_t i = 0; i < decisions.size(); ++i) {
        const Kumipuyo& kp = seq.get(i % seq.size());
        int d = 0;
        switch (kp.axis) {
        case PuyoColor::RED:
            d += 0;
            break;
        case PuyoColor::GREEN:
            d += 1 * 5;
            break;
        case PuyoColor::BLUE:
            d += 2 * 5;
            break;
        case PuyoColor::YELLOW:
            d += 3 * 5;
            break;
        default:
            CHECK(false);
        }

        switch (kp.child) {
        case PuyoColor::RED:
            d += 0;
            break;
        case PuyoColor::GREEN:
            d += 1;
            break;
        case PuyoColor::BLUE:
            d += 2;
            break;
        case PuyoColor::YELLOW:
            d += 3;
            break;
        default:
            CHECK(false);
        }

        if (i < decisions.size()) {
            int h = (decisions[i].x << 2) + decisions[i].r;
            d |= h << 7;
        }

        ss << ENCODER[d & 0x3F] << ENCODER[(d >> 6) & 0x3F];
    }

    return ss.str();
}
