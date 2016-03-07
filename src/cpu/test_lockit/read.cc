#include "read.h"

#include <cstring>

#include "core/puyo_color.h"
#include "field.h"

using namespace std;

namespace test_lockit {

void READ_P::ref()
{
    int i, j;
    for (i = 0; i < 6; i++) {
        for (j = 0; j < kHeight; j++) {
            field[i][j] = PuyoColor::EMPTY;
            yosou_field[i][j] = PuyoColor::EMPTY;
        }
        tsumo[i] = PuyoColor::RED; // Why RED here?
        field12[i] = PuyoColor::EMPTY;
    }
    act_on = 0;
    act_on_1st = 0;
    nex_on = 0;
    set_puyo = 0;
    set_puyo_once = 1;
    rensa_end = 0;
    rensa_end_once = 0;
    score = 0;
    keep_score = 0;
    zenkesi = 0;
    id = 0;
    te_x = 3;
    te_r = 0;
}

READ_P::READ_P()
{
    int i, j;
    for (i = 0; i < 6; i++) {
        for (j = 0; j < kHeight; j++) {
            field[i][j] = PuyoColor::EMPTY;
            yosou_field[i][j] = PuyoColor::EMPTY;
        }
        tsumo[i] = PuyoColor::RED;
        field12[i] = PuyoColor::EMPTY;
    }
    act_on = 0;
    act_on_1st = 0;
    nex_on = 0;
    set_puyo = 0;
    set_puyo_once = 1;
    rensa_end = 0;
    rensa_end_once = 0;
    score = 0;
    keep_score = 0;
    zenkesi = 0;
    id = 0;
    te_x = 3;
    te_r = 0;
}

READ_P::~READ_P()
{
}

void READ_P::fall()
{
    int i, j, n;
    for (i = 0; i < 6; i++) {
        n = 0;
        for (j = 0; j < 13; j++) {
            if (field[i][j] == PuyoColor::EMPTY) {
                n++;
            } else if (n != 0) {
                field[i][j - n] = field[i][j];
                field[i][j] = PuyoColor::EMPTY;
            }
        }
    }
}

void READ_P::setti_12()
{
    for (int i = 0; i < 6; i++) {
        field[i][12] = field12[i];
    }
}

void READ_P::field_kioku()
{
    memcpy(field_hoz, field, sizeof(field_hoz));
}

int READ_P::field_hikaku()
{
    int count = 0;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 12; j++) {
            if (field[i][j] != yosou_field[i][j]) {
                count++;
            }
        }
    }
    return count;
}

int READ_P::setti_puyo()
{
    int i, j;
    int aa = 0;
    PuyoColor nx1, nx2;
    int modori = 0;
    setti_basyo[0] = -1;
    setti_basyo[1] = -1;
    setti_basyo[2] = -1;
    setti_basyo[3] = -1;
    if (te_r == 0)
        aa = 0;
    if (te_r == 1)
        aa = 17;
    if (te_r == 2)
        aa = 6;
    if (te_r == 3)
        aa = 12 - 1;
    aa = aa + te_x - 1;
    nx1 = tsumo[0];
    nx2 = tsumo[1];
    if (aa < 6) {
        for (j = 0; j < 13; j++) {
            if (field[aa][j] == PuyoColor::EMPTY) {
                field[aa][j] = nx1;
                field[aa][j + 1] = nx2;
                setti_basyo[0] = aa;
                setti_basyo[1] = j;
                setti_basyo[2] = aa;
                setti_basyo[3] = j + 1;
                break;
            }
        }
    } else if (aa < 12) {
        for (j = 0; j < 13; j++) {
            if (field[aa - 6][j] == PuyoColor::EMPTY) {
                field[aa - 6][j] = nx2;
                field[aa - 6][j + 1] = nx1;
                setti_basyo[0] = aa - 6;
                setti_basyo[1] = j;
                setti_basyo[2] = aa - 6;
                setti_basyo[3] = j + 1;
                break;
            }
        }
    } else if (aa < 17) {
        for (j = 0; j < 13; j++) {
            if (field[aa - 11][j] == PuyoColor::EMPTY) {
                field[aa - 11][j] = nx1;
                setti_basyo[0] = aa - 11;
                setti_basyo[1] = j;
                break;
            }
        }
        for (j = 0; j < 13; j++) {
            if (field[aa - 12][j] == PuyoColor::EMPTY) {
                field[aa - 12][j] = nx2;
                setti_basyo[2] = aa - 12;
                setti_basyo[3] = j;
                break;
            }
        }
    } else if (aa < 22) {
        for (j = 0; j < 13; j++) {
            if (field[aa - 17][j] == PuyoColor::EMPTY) {
                field[aa - 17][j] = nx1;
                setti_basyo[0] = aa - 17;
                setti_basyo[1] = j;
                break;
            }
        }
        for (j = 0; j < 13; j++) {
            if (field[aa - 16][j] == PuyoColor::EMPTY) {
                field[aa - 16][j] = nx2;
                setti_basyo[2] = aa - 16;
                setti_basyo[3] = j;
                break;
            }
        }
    }
    modori = chousei_syoukyo();
    for (i = 0; i < 6; i++) {
        field12[i] = field[i][12];
        for (j = 0; j < kHeight; j++) {
            yosou_field[i][j] = field[i][j];
        }
    }
    return modori;
}

int READ_P::chousei_syoukyo()
{
    int num = 0;
    int numa = 0;
    int numb = 0;
    Check point[6][12] {};
    int i, j;
    int syo = 1;
    int kiept[6] = { 0 };
    int rakkaflg[6] = { 0 };
    int n;
    int a, b, c, d;

    a = setti_basyo[0];
    b = setti_basyo[1];
    c = setti_basyo[2];
    d = setti_basyo[3];
    if ((b < 12) && (b >= 0)) {
        saiki(field, point, a, b, &numa, field[a][b]);
    }
    if ((d < 12) && (d >= 0)) {
        if (point[c][d] == Check::Unchecked) {
            saiki(field, point, c, d, &numb, field[c][d]);
        }
    }
    if ((numa < 4) && (numb < 4))
        return 0;
    if (numa > 3) {
        syou(field, a, b, field[a][b], rakkaflg);
    }
    if (numb > 3) {
        syou(field, c, d, field[c][d], rakkaflg);
    }

    for (i = 0; i < 6; i++) {
        kiept[i] = 12;
        if (rakkaflg[i] == 1) {
            n = 0;
            for (j = 0; j < 13; j++) {
                if (field[i][j] == PuyoColor::EMPTY) {
                    if (n == 0)
                        kiept[i] = j;
                    n++;
                } else if (n != 0) {
                    field[i][j - n] = field[i][j];
                    field[i][j] = PuyoColor::EMPTY;
                }
            }
        }
    }

    while (syo) {
        syo = 0;
        std::fill_n(&point[0][0], 6 * 12, Check::Unchecked);
        rakkaflg[0] = 0;
        rakkaflg[1] = 0;
        rakkaflg[2] = 0;
        rakkaflg[3] = 0;
        rakkaflg[4] = 0;
        rakkaflg[5] = 0;
        for (i = 0; i < 6; i++) {
            for (j = kiept[i]; j < 12; j++) {
                if (field[i][j] == PuyoColor::EMPTY)
                    continue;
                if ((point[i][j] != Check::Checked) && (field[i][j] != PuyoColor::OJAMA)) {
                    saiki(field, point, i, j, &num, field[i][j]);
                    if (num > 3) {
                        syo = 1;
                        syou(field, i, j, field[i][j], rakkaflg);
                    }
                    num = 0;
                }
            }
        }
        for (i = 0; i < 6; i++) {
            kiept[i] = 12;
            if (rakkaflg[i] == 1) {
                n = 0;
                for (j = 0; j < 13; j++) {
                    if (field[i][j] == PuyoColor::EMPTY) {
                        if (n == 0)
                            kiept[i] = j;
                        n++;
                    } else if (n != 0) {
                        field[i][j - n] = field[i][j];
                        field[i][j] = PuyoColor::EMPTY;
                    }
                }
            }
        }
    }
    return 1;
}

}  // namespace test_lockit
