#include "field.h"

#include <glog/logging.h>

#include "base/small_int_set.h"
#include "core/puyo_color.h"
#include "core/field_constant.h"
#include "core/score.h"
#include "rensa_result.h"

namespace test_lockit {

namespace {

void syou_right(PuyoColor[][kHeight], int, int, PuyoColor, int[]);
void syou_left(PuyoColor[][kHeight], int, int, PuyoColor, int[]);
void syou_up(PuyoColor[][kHeight], int, int, PuyoColor, int[]);
void syou_down(PuyoColor[][kHeight], int, int, PuyoColor, int[]);

void syou_right_num(PuyoColor[][kHeight], int, int, PuyoColor, int[], int*);
void syou_left_num(PuyoColor[][kHeight], int, int, PuyoColor, int[], int*);
void syou_up_num(PuyoColor[][kHeight], int, int, PuyoColor, int[], int*);
void syou_down_num(PuyoColor[][kHeight], int, int, PuyoColor, int[], int*);

void syou_right(PuyoColor ba[][kHeight], int x, int y, PuyoColor incol, int flg[])
{
  ba[x][y] = PuyoColor::EMPTY;
  flg[x] = 1;
  if ((y != 11) && (ba[x][y + 1] == incol))
    syou_up(ba, x, y + 1, incol, flg);
  if ((y != 11) && (ba[x][y + 1] == PuyoColor::OJAMA))
    ba[x][y + 1] = PuyoColor::EMPTY;
  if ((x != 5) && (ba[x + 1][y] == incol))
    syou_right(ba, x + 1, y, incol, flg);
  if ((x != 5) && (ba[x + 1][y] == PuyoColor::OJAMA)) {
    ba[x + 1][y] = PuyoColor::EMPTY;
    flg[x + 1] = 1;
  }
  if ((y != 0) && (ba[x][y - 1] == incol))
    syou_down(ba, x, y - 1, incol, flg);
  if ((y != 0) && (ba[x][y - 1] == PuyoColor::OJAMA))
    ba[x][y - 1] = PuyoColor::EMPTY;
}

void syou_left(PuyoColor ba[][kHeight], int x, int y, PuyoColor incol, int flg[])
{
    ba[x][y] = PuyoColor::EMPTY;
    flg[x] = 1;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left(ba, x - 1, y, incol, flg);
    if ((x != 0) && (ba[x - 1][y] == PuyoColor::OJAMA)) {
        ba[x - 1][y] = PuyoColor::EMPTY;
        flg[x - 1] = 1;
    }
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up(ba, x, y + 1, incol, flg);
    if ((y != 11) && (ba[x][y + 1] == PuyoColor::OJAMA))
        ba[x][y + 1] = PuyoColor::EMPTY;
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down(ba, x, y - 1, incol, flg);
    if ((y != 0) && (ba[x][y - 1] == PuyoColor::OJAMA))
        ba[x][y - 1] = PuyoColor::EMPTY;
}

void syou_up(PuyoColor ba[][kHeight], int x, int y, PuyoColor incol, int flg[])
{
    ba[x][y] = PuyoColor::EMPTY;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left(ba, x - 1, y, incol, flg);
    if ((x != 0) && (ba[x - 1][y] == PuyoColor::OJAMA)) {
        ba[x - 1][y] = PuyoColor::EMPTY;
        flg[x - 1] = 1;
    }
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up(ba, x, y + 1, incol, flg);
    if ((y != 11) && (ba[x][y + 1] == PuyoColor::OJAMA))
        ba[x][y + 1] = PuyoColor::EMPTY;
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right(ba, x + 1, y, incol, flg);
    if ((x != 5) && (ba[x + 1][y] == PuyoColor::OJAMA)) {
        ba[x + 1][y] = PuyoColor::EMPTY;
        flg[x + 1] = 1;
    }
}

void syou_down(PuyoColor ba[][kHeight], int x, int y, PuyoColor incol, int flg[])
{
    ba[x][y] = PuyoColor::EMPTY;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left(ba, x - 1, y, incol, flg);
    if ((x != 0) && (ba[x - 1][y] == PuyoColor::OJAMA)) {
        ba[x - 1][y] = PuyoColor::EMPTY;
        flg[x - 1] = 1;
    }
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right(ba, x + 1, y, incol, flg);
    if ((x != 5) && (ba[x + 1][y] == PuyoColor::OJAMA)) {
        ba[x + 1][y] = PuyoColor::EMPTY;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down(ba, x, y - 1, incol, flg);
    if ((y != 0) && (ba[x][y - 1] == PuyoColor::OJAMA))
        ba[x][y - 1] = PuyoColor::EMPTY;
}

void syou_right_num(PuyoColor ba[][kHeight], int x, int y, PuyoColor incol, int flg[], int* num)
{
    ba[x][y] = PuyoColor::EMPTY;
    flg[x] = 1;
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up_num(ba, x, y + 1, incol, flg, num);
    if ((y != 11) && (ba[x][y + 1] == PuyoColor::OJAMA))
        ba[x][y + 1] = PuyoColor::EMPTY;
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right_num(ba, x + 1, y, incol, flg, num);
    if ((x != 5) && (ba[x + 1][y] == PuyoColor::OJAMA)) {
        ba[x + 1][y] = PuyoColor::EMPTY;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down_num(ba, x, y - 1, incol, flg, num);
    if ((y != 0) && (ba[x][y - 1] == PuyoColor::OJAMA))
        ba[x][y - 1] = PuyoColor::EMPTY;
}

void syou_left_num(PuyoColor ba[][kHeight], int x, int y, PuyoColor incol, int flg[], int* num)
{
    ba[x][y] = PuyoColor::EMPTY;
    flg[x] = 1;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left_num(ba, x - 1, y, incol, flg, num);
    if ((x != 0) && (ba[x - 1][y] == PuyoColor::OJAMA)) {
        ba[x - 1][y] = PuyoColor::EMPTY;
        flg[x - 1] = 1;
    }
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up_num(ba, x, y + 1, incol, flg, num);
    if ((y != 11) && (ba[x][y + 1] == PuyoColor::OJAMA))
        ba[x][y + 1] = PuyoColor::EMPTY;
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down_num(ba, x, y - 1, incol, flg, num);
    if ((y != 0) && (ba[x][y - 1] == PuyoColor::OJAMA))
        ba[x][y - 1] = PuyoColor::EMPTY;
}

void syou_up_num(PuyoColor ba[][kHeight], int x, int y, PuyoColor incol, int flg[], int* num)
{
    ba[x][y] = PuyoColor::EMPTY;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left_num(ba, x - 1, y, incol, flg, num);
    if ((x != 0) && (ba[x - 1][y] == PuyoColor::OJAMA)) {
        ba[x - 1][y] = PuyoColor::EMPTY;
        flg[x - 1] = 1;
    }
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up_num(ba, x, y + 1, incol, flg, num);
    if ((y != 11) && (ba[x][y + 1] == PuyoColor::OJAMA))
        ba[x][y + 1] = PuyoColor::EMPTY;
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right_num(ba, x + 1, y, incol, flg, num);
    if ((x != 5) && (ba[x + 1][y] == PuyoColor::OJAMA)) {
        ba[x + 1][y] = PuyoColor::EMPTY;
        flg[x + 1] = 1;
    }
}

void syou_down_num(PuyoColor ba[][kHeight], int x, int y, PuyoColor incol, int flg[], int* num)
{
    ba[x][y] = PuyoColor::EMPTY;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left_num(ba, x - 1, y, incol, flg, num);
    if ((x != 0) && (ba[x - 1][y] == PuyoColor::OJAMA)) {
        ba[x - 1][y] = PuyoColor::EMPTY;
        flg[x - 1] = 1;
    }
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right_num(ba, x + 1, y, incol, flg, num);
    if ((x != 5) && (ba[x + 1][y] == PuyoColor::OJAMA)) {
        ba[x + 1][y] = PuyoColor::EMPTY;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down_num(ba, x, y - 1, incol, flg, num);
    if ((y != 0) && (ba[x][y - 1] == PuyoColor::OJAMA))
        ba[x][y - 1] = PuyoColor::EMPTY;
}

} // namespace

bool isTLFieldEmpty(const PuyoColor field[6][kHeight])
{
    for (int i = 0; i < 6; ++i) {
        if (field[i][0] != PuyoColor::EMPTY) {
            return false;
        }
    }
    return true;
}

int countNormalColor13(const PuyoColor f[][kHeight])
{
    int n = 0;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 13; j++) {
            if (isNormalColor(f[i][j]))
                ++n;
        }
    }

    return n;
}

void copyField(const PuyoColor src[][kHeight], PuyoColor dst[][kHeight])
{
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < kHeight; ++j) {
            dst[i][j] = src[i][j];
        }
    }
}

TLRensaResult simulate(PuyoColor field[][kHeight])
{
    // Assume all puyos are grounded.
    // TODO: add DCHECK to check it.

    // parameters necessary to compute score
    int chain = 0;
    int long_bonus[TLRensaResult::MAX_RENSA] {};
    int num_colors[TLRensaResult::MAX_RENSA] {};
    int num_connected[TLRensaResult::MAX_RENSA] {};   // # of connected puyos
    int num_connections[TLRensaResult::MAX_RENSA] {};  // # of groups
    bool quick = false;

    int bottom[6] {};
    bool cont = true;
    while (cont) {
        cont = false;
        Check point[6][12] {};
        int rakkaflg[6] {};

        SmallIntSet used_colors;
        // check connections and vanish
        for (int i = 0; i < 6; ++i) {
            for (int j = bottom[i]; j < 12; ++j) {
                PuyoColor color = field[i][j];
                if (color == PuyoColor::EMPTY)
                    continue;
                if (point[i][j] != Check::Checked && isNormalColor(color)) {
                    int num = 0;
                    saiki(field, point, i, j, &num, color);
                    if (num >= 4) {
                        cont = true;
                        syou(field, i, j, color, rakkaflg);

                        long_bonus[chain] += longBonus(num);
                        num_connected[chain] += num;
                        used_colors.set(ordinal(color));
                        num_connections[chain]++;
                    }
                }
            }
        }
        num_colors[chain] = used_colors.size();
        if (!cont)
            break;
        ++chain;

        // drop puyos
        quick = true;
        for (int i = 0; i < 6; ++i) {
            bottom[i] = 12;
            if (rakkaflg[i]) {
                int n = 0;
                for (int j = 0; j < 13; ++j) {
                    if (field[i][j] == PuyoColor::EMPTY) {
                        if (n == 0)
                            bottom[i] = j;
                        n++;
                    } else if (n != 0) {
                        field[i][j - n] = field[i][j];
                        field[i][j] = PuyoColor::EMPTY;
                        quick = false;
                    }
                }
            }
        }
    }

    int score = 0;
    int num_vanished = 0;
    for (int i = 0; i < chain; ++i) {
        int chain_bonus = chainBonus(i + 1);
        int color_bonus = colorBonus(num_colors[i]);
        int rate = calculateRensaBonusCoef(chain_bonus, long_bonus[i], color_bonus);
        score += num_connected[i] * rate * 10;
        num_vanished += num_connected[i];
    }

    TLRensaResult result;
    result.chains = chain;
    result.score = score;
    result.quick = quick;
    result.num_vanished = num_vanished;
    for (int i = 0; i < chain; ++i) {
        result.num_connections[i] = num_connections[i];
    }

    return result;
}

void saiki(const PuyoColor ba[][kHeight], Check point[][12], int x, int y, int* num, PuyoColor incol)
{
    point[x][y] = Check::Checked;
    *num += 1;
    if ((x != 0) && (incol == ba[x - 1][y]) && (point[x - 1][y] != Check::Checked))
        saiki(ba, point, x - 1, y, num, incol);
    if ((y != 11) && (incol == ba[x][y + 1]) && (point[x][y + 1] != Check::Checked))
        saiki(ba, point, x, y + 1, num, incol);
    if ((x != 5) && (incol == ba[x + 1][y]) && (point[x + 1][y] != Check::Checked))
        saiki(ba, point, x + 1, y, num, incol);
    if ((y != 0) && (incol == ba[x][y - 1]) && (point[x][y - 1] != Check::Checked))
        saiki(ba, point, x, y - 1, num, incol);
}

void saiki_4(PuyoColor ba[][kHeight], int x, int y, int* num, PuyoColor incol)
{
    ba[x][y] = PuyoColor::EMPTY;
    *num += 1;
    if (*num > 3) {
        ba[x][y] = incol;
        return;
    }
    if ((x != 0) && (incol == ba[x - 1][y]))
        saiki_4(ba, x - 1, y, num, incol);
    if ((y != 11) && (incol == ba[x][y + 1]))
        saiki_4(ba, x, y + 1, num, incol);
    if ((x != 5) && (incol == ba[x + 1][y]))
        saiki_4(ba, x + 1, y, num, incol);
    if ((y != 0) && (incol == ba[x][y - 1]))
        saiki_4(ba, x, y - 1, num, incol);
    ba[x][y] = incol;
}

void syou(PuyoColor ba[][kHeight], int x, int y, PuyoColor incol, int flg[])
{
    ba[x][y] = PuyoColor::EMPTY;
    flg[x] = 1;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left(ba, x - 1, y, incol, flg);
    if ((x != 0) && (ba[x - 1][y] == PuyoColor::OJAMA)) {
        ba[x - 1][y] = PuyoColor::EMPTY;
        flg[x - 1] = 1;
    }
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up(ba, x, y + 1, incol, flg);
    if ((y != 11) && (ba[x][y + 1] == PuyoColor::OJAMA))
        ba[x][y + 1] = PuyoColor::EMPTY;
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right(ba, x + 1, y, incol, flg);
    if ((x != 5) && (ba[x + 1][y] == PuyoColor::OJAMA)) {
        ba[x + 1][y] = PuyoColor::EMPTY;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down(ba, x, y - 1, incol, flg);
    if ((y != 0) && (ba[x][y - 1] == PuyoColor::OJAMA))
        ba[x][y - 1] = PuyoColor::EMPTY;
}

void syou_downx(PuyoColor ba[][kHeight], int x, int y, PuyoColor incol, int flg[], int* num)
{
    *num += 1;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left_num(ba, x - 1, y, incol, flg, num);
    if ((x != 0) && (ba[x - 1][y] == PuyoColor::OJAMA)) {
        ba[x - 1][y] = PuyoColor::EMPTY;
        flg[x - 1] = 1;
    }
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right_num(ba, x + 1, y, incol, flg, num);
    if ((x != 5) && (ba[x + 1][y] == PuyoColor::OJAMA)) {
        ba[x + 1][y] = PuyoColor::EMPTY;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down_num(ba, x, y - 1, incol, flg, num);
    if ((y != 0) && (ba[x][y - 1] == PuyoColor::OJAMA))
        ba[x][y - 1] = PuyoColor::EMPTY;
}

bool setti_puyo(PuyoColor ba[][kHeight], int aa, PuyoColor nx1, PuyoColor nx2, int setti_basyo[])
{
    if (aa < 6) {
        setti_basyo[0] = aa;
        setti_basyo[1] = -1;
        setti_basyo[2] = aa;
        setti_basyo[3] = -1;
        for (int j = 0; j < 13; j++) {
            if (ba[aa][j] == PuyoColor::EMPTY) {
                ba[aa][j] = nx1;
                ba[aa][j + 1] = nx2;
                setti_basyo[1] = j;
                setti_basyo[3] = j + 1;
                return true;
            }
        }

        return false;
    }

    if (aa < 12) {
        setti_basyo[0] = aa - 6;
        setti_basyo[1] = -1;
        setti_basyo[2] = aa - 6;
        setti_basyo[3] = -1;
        for (int j = 0; j < 13; j++) {
            if (ba[aa - 6][j] == PuyoColor::EMPTY) {
                ba[aa - 6][j] = nx2;
                ba[aa - 6][j + 1] = nx1;
                setti_basyo[1] = j;
                setti_basyo[3] = j + 1;
                return true;
            }
        }

        return false;
    }

    if (aa < 17) {
        setti_basyo[0] = aa - 11;
        setti_basyo[1] = -1;
        setti_basyo[2] = aa - 12;
        setti_basyo[3] = -1;
        for (int j = 0; j < 13; j++) {
            if (ba[aa - 11][j] == PuyoColor::EMPTY) {
                setti_basyo[1] = j;
                break;
            }
        }
        for (int j = 0; j < 13; j++) {
            if (ba[aa - 12][j] == PuyoColor::EMPTY) {
                setti_basyo[3] = j;
                break;
            }
        }

        if (setti_basyo[1] >= 0 && setti_basyo[3] >= 0) {
            ba[aa - 11][setti_basyo[1]] = nx1;
            ba[aa - 12][setti_basyo[3]] = nx2;
            return true;
        }

        return false;
    }

    if (aa < 22) {
        setti_basyo[0] = aa - 17;
        setti_basyo[1] = -1;
        setti_basyo[2] = aa - 16;
        setti_basyo[3] = -1;
        for (int j = 0; j < 13; j++) {
            if (ba[aa - 17][j] == PuyoColor::EMPTY) {
                setti_basyo[1] = j;
                break;
            }
        }
        for (int j = 0; j < 13; j++) {
            if (ba[aa - 16][j] == PuyoColor::EMPTY) {
                setti_basyo[3] = j;
                break;
            }
        }

        if (setti_basyo[1] >= 0 && setti_basyo[3] >= 0) {
            ba[aa - 17][setti_basyo[1]] = nx1;
            ba[aa - 16][setti_basyo[3]] = nx2;
            return true;
        }

        return false;
    }

    CHECK(false) << "Invalid aa=" << aa;
    return false;
}

int setti_puyo_1(PuyoColor ba[][kHeight], int eex, PuyoColor eecol)
{
    int j;
    int oita = 0;
    int num = 0;
    int setti_basyoy;
    for (j = 0; j < 12; j++) {
        if (ba[eex][j] == PuyoColor::EMPTY) {
            ba[eex][j] = eecol;
            setti_basyoy = j;
            oita = 1;
            break;
        }
    }
    if (oita == 0)
        return 1;
    saiki_4(ba, eex, setti_basyoy, &num, eecol);
    if (num > 3)
        return 1;

    return 0;
}

int chousei_syoukyo_3(PuyoColor bass[][kHeight], int[], int* poi2s, int* score, Check tokus, int i2, int j2, int ruiseki_point)
{
    int rensa_rate[19] = { 0, 8, 16, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480, 512 };
    int color_rate[5] = { 0, 3, 6, 12, 24 };
    int renketsu[19][NUM_PUYO_COLORS] {};
    int colnum;
    int renketsunum;
    int renketsubonus[19] = { 0 };
    int rate;

    int num = 0;
    Check point[6][12] {};
    int i, j;
    int syo = 1;
    int kiept[6] = { 0 };
    int rakkaflg[6] = { 0 };
    int n;
    int chain = 1;
    int rakka_ruiseki = 0;
    *score = 0;
    rakkaflg[0] = 0;
    rakkaflg[1] = 0;
    rakkaflg[2] = 0;
    rakkaflg[3] = 0;
    rakkaflg[4] = 0;
    rakkaflg[5] = 0;
    PuyoColor color = bass[i2][j2];
    switch (tokus) {
    case Check::Unchecked:
    case Check::Checked:
    case Check::ColorWithEmptyUR:
    case Check::ColorWithEmptyUL:
    case Check::ColorWithEmptyU:
        syou_downx(bass, i2, j2 + 1, bass[i2][j2], rakkaflg, &num);
        break;
    case Check::ColorWithEmptyLR:
    case Check::ColorWithEmptyL:
        syou_downx(bass, i2 + 1, j2, bass[i2][j2], rakkaflg, &num);
        break;
    case Check::ColorWithEmptyR:
        syou_downx(bass, i2 - 1, j2, bass[i2][j2], rakkaflg, &num);
        break;
    case Check::Unknown:
    case Check::Empty:
        ; // do nothing
    }
    renketsu[0][ordinal(color)] = num;
    if (num > 10)
        renketsubonus[0] += 10; // bugggggg 111102
    else if (num > 4)
        renketsubonus[0] = num - 3;

    num = 0;

    kiept[0] = 0;
    kiept[1] = 0;
    kiept[2] = 0;
    kiept[3] = 0;
    kiept[4] = 0;
    kiept[5] = 0;
    for (i = 0; i < 6; i++) {
        kiept[i] = 12;
        if (rakkaflg[i] == 1) {
            n = 0;
            for (j = 0; j < 13; j++) {
                if (bass[i][j] == PuyoColor::EMPTY) {
                    if (n == 0)
                        kiept[i] = j;
                    n++;
                } else if (n != 0) {
                    bass[i][j - n] = bass[i][j];
                    bass[i][j] = PuyoColor::EMPTY;
                }
            }
            rakka_ruiseki += n;
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
                if (point[i][j] != Check::Unchecked)
                    continue;
                if (bass[i][j] == PuyoColor::EMPTY)
                    break;
                if (bass[i][j] != PuyoColor::OJAMA) {
                    saiki(bass, point, i, j, &num, bass[i][j]);
                    if (num > 3) {
                        syo = 1;
                        color = bass[i][j];
                        renketsu[chain][ordinal(color)] += num;
                        if (num > 10)
                            renketsubonus[chain] += 10; // bugggggg 111102
                        else if (num > 4)
                            renketsubonus[chain] += num - 3;
                        (*poi2s) = (*poi2s) - num * num;
                        syou(bass, i, j, bass[i][j], rakkaflg);
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
                    if (bass[i][j] == PuyoColor::EMPTY) {
                        if (n == 0)
                            kiept[i] = j;
                        n++;
                    } else if (n != 0) {
                        bass[i][j - n] = bass[i][j];
                        bass[i][j] = PuyoColor::EMPTY;
                    }
                }
                rakka_ruiseki += n;
            }
        }
        chain++;
    } // while
    chain--;

    for (i = 0; i < (chain); i++) {
        rate = 0;
        colnum = 0;
        renketsunum = 0;
        for (j = 0; j < 5; j++) {
            colnum += (renketsu[i][j] != 0);
            renketsunum += renketsu[i][j];
        }
        rate = color_rate[colnum - 1] + renketsubonus[i] + rensa_rate[i];
        if (rate == 0)
            rate = 1;
        *score += renketsunum * rate * 10;
    }
    (*poi2s) = (*poi2s) - rakka_ruiseki * ruiseki_point;
    return chain;
}

int chousei_syoukyo_sc(PuyoColor field[][kHeight], int[], int* score)
{
    TLRensaResult result = simulate(field);
    *score = result.score;
    return result.num_vanished;
}

void setti_ojama(PuyoColor f[][kHeight], int numOjama)
{
    int lines = (numOjama + 3) / 6;

    for (int i = 0; i < 6; i++) {
        int cnt = 0;
        for (int j = 0; j < 13; j++) {
            if (f[i][j] == PuyoColor::EMPTY) {
                f[i][j] = PuyoColor::OJAMA;
                cnt++;
            }

            if (cnt == lines)
                break;
        }
    }
}

}  // namespace test_lockit
