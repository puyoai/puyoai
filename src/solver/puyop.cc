#include "solver/puyop.h"

#include <sstream>

#include "core/core_field.h"
#include "core/decision.h"
#include "core/kumipuyo_seq.h"

using namespace std;

namespace {

const char URL_PREFIX[] = "http://www.puyop.com/s/";

// 64 characters
const char ENCODER[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]";

inline int tsumoColorId(const PuyoColor& c)
{
    switch (c) {
    case PuyoColor::RED:    return 0;
    case PuyoColor::GREEN:  return 1;
    case PuyoColor::BLUE:   return 2;
    case PuyoColor::YELLOW: return 3;
    // PURPLE -> return 4
    default: CHECK(false);
  }
  CHECK(false);
  return 0;
}

string encodeControl(const KumipuyoSeq& seq, const vector<Decision>& decisions)
{
    stringstream ss;
    for (size_t i = 0; i < decisions.size(); ++i) {
        const Kumipuyo& kp = seq.get(i % seq.size());
        int d = tsumoColorId(kp.axis) * 5 + tsumoColorId(kp.child);
        int h = (decisions[i].x << 2) + decisions[i].r;
        d |= h << 7;

        ss << ENCODER[d & 0x3F] << ENCODER[(d >> 6) & 0x3F];
    }
    return ss.str();
}

inline int fieldColorId(const PuyoColor& c)
{
    switch (c) {
    case PuyoColor::EMPTY:  return 0;
    case PuyoColor::RED:    return 1;
    case PuyoColor::GREEN:  return 2;
    case PuyoColor::BLUE:   return 3;
    case PuyoColor::YELLOW: return 4;
    // PURPLE -> return 5
    case PuyoColor::OJAMA:  return 6;
    // Do not support KATAPUYO, IRON, and WALL, becuase they need irregular encoding.
    default: CHECK(false);
  }
  CHECK(false);
  return 0;
}

string encodeField(const CoreField& field)
{
    if (field.isZenkeshi()) {
        return "";
    }
  
    stringstream ss;
    bool start = false;
    for (int y = 13; y > 0; --y) {
        for (int x = 1; x <= 6; x += 2) {
            if (!start && field.isEmpty(x, y) && field.isEmpty(x + 1, y)) {
                continue;
            }

            int d = fieldColorId(field.color(x, y)) * 8 + fieldColorId(field.color(x + 1, y));
            CHECK_GT(64, d);
            CHECK_LE(0, d);
            start = true;
            ss << ENCODER[d];
        }
    }
    return ss.str();
}

}  // namespace

string makePuyopURL(const KumipuyoSeq& seq, const vector<Decision>& decisions)
{
    return makePuyopURL(CoreField(), seq, decisions);
}

string makePuyopURL(const CoreField& field, const KumipuyoSeq& seq, const vector<Decision>& decisions)
{
    stringstream ss;
    ss << URL_PREFIX << encodeField(field) << "_" << encodeControl(seq, decisions);
    return ss.str();
}
