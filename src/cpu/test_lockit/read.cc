#include "read.h"

#include <cstring>

#include "core/core_field.h"
#include "core/kumipuyo.h"
#include "core/puyo_color.h"
#include "field.h"

using namespace std;

namespace test_lockit {

void READ_P::ref()
{
    field = CoreField();
    yosou_field = CoreField();
    for (int i = 0; i < 6; i++) {
        tsumo[i] = PuyoColor::RED; // Why RED here?
        field12[i] = PuyoColor::EMPTY;
    }
    act_on_1st = 0;
    set_puyo_once = 1;
    rensa_end_once = 0;
    score = 0;
    keep_score = 0;
    zenkesi = 0;
    id = 0;
    decision = Decision(3, 0);
}

READ_P::READ_P()
{
    field = CoreField();
    yosou_field = CoreField();
    for (int i = 0; i < 6; i++) {
        tsumo[i] = PuyoColor::RED;
        field12[i] = PuyoColor::EMPTY;
    }
    act_on_1st = 0;
    set_puyo_once = 1;
    rensa_end_once = 0;
    score = 0;
    keep_score = 0;
    zenkesi = 0;
    id = 0;
    decision = Decision(3, 0);
}

READ_P::~READ_P()
{
}

void READ_P::setti_12()
{
    for (int i = 0; i < 6; i++) {
        if (field12[i] != PuyoColor::EMPTY)
            field.dropPuyoOn(i + 1, field12[i]);
    }
}

void READ_P::field_kioku()
{
    field_hoz = field;
}

int READ_P::field_hikaku()
{
    int count = 0;
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 12; ++y) {
            if (field.color(x, y) != yosou_field.color(x, y)) {
                ++count;
            }
        }
    }
    return count;
}

int READ_P::setti_puyo()
{
    PuyoColor nx1 = tsumo[0];
    PuyoColor nx2 = tsumo[1];
    field.dropKumipuyo(decision, Kumipuyo(nx1, nx2));
    RensaResult result = field.simulate();
    yosou_field = field;
    return (result.chains > 0) ? 1 : 0;
}

}  // namespace test_lockit
