#include "field.h"

#include <glog/logging.h>

#include "base/small_int_set.h"
#include "color.h"
#include "core/field_constant.h"
#include "core/score.h"
#include "rensa_result.h"

namespace test_lockit {

namespace {

void saiki_right(const TLColor[][kHeight], int[][12], int, int, int*, TLColor);
void saiki_left(const TLColor[][kHeight], int[][12], int, int, int*, TLColor);
void saiki_up(const TLColor[][kHeight], int[][12], int, int, int*, TLColor);
void saiki_down(const TLColor[][kHeight], int[][12], int, int, int*, TLColor);

void saiki_3_right(const TLColor[][kHeight], int[][12], int, int, int*, TLColor);
void saiki_3_left(const TLColor[][kHeight], int[][12], int, int, int*, TLColor);
void saiki_3_up(const TLColor[][kHeight], int[][12], int, int, int*, TLColor);
void saiki_3_down(const TLColor[][kHeight], int[][12], int, int, int*, TLColor);

void syou_right(TLColor[][kHeight], int, int, TLColor, int[]);
void syou_left(TLColor[][kHeight], int, int, TLColor, int[]);
void syou_up(TLColor[][kHeight], int, int, TLColor, int[]);
void syou_down(TLColor[][kHeight], int, int, TLColor, int[]);

void syou_right_num(TLColor[][kHeight], int, int, TLColor, int[], int*);
void syou_left_num(TLColor[][kHeight], int, int, TLColor, int[], int*);
void syou_up_num(TLColor[][kHeight], int, int, TLColor, int[], int*);
void syou_down_num(TLColor[][kHeight], int, int, TLColor, int[], int*);

void saiki_right(const TLColor ba[][kHeight], int point[][12], int x, int y, int* num, TLColor incol)
{
    point[x][y] = 1;
    *num += 1;
    if ((y != 11) && (incol == ba[x][y + 1]) && (point[x][y + 1] == 0))
        saiki_up(ba, point, x, y + 1, num, incol);
    if ((x != 5) && (incol == ba[x + 1][y]) && (point[x + 1][y] == 0))
        saiki_right(ba, point, x + 1, y, num, incol);
    if ((y != 0) && (incol == ba[x][y - 1]) && (point[x][y - 1] == 0))
        saiki_down(ba, point, x, y - 1, num, incol);
}

void saiki_left(const TLColor ba[][kHeight], int point[][12], int x, int y, int* num, TLColor incol)
{
    point[x][y] = 1;
    *num += 1;
    if ((x != 0) && (incol == ba[x - 1][y]) && (point[x - 1][y] == 0))
        saiki_left(ba, point, x - 1, y, num, incol);
    if ((y != 11) && (incol == ba[x][y + 1]) && (point[x][y + 1] == 0))
        saiki_up(ba, point, x, y + 1, num, incol);
    if ((y != 0) && (incol == ba[x][y - 1]) && (point[x][y - 1] == 0))
        saiki_down(ba, point, x, y - 1, num, incol);
}

void saiki_up(const TLColor ba[][kHeight], int point[][12], int x, int y, int* num, TLColor incol)
{
    point[x][y] = 1;
    *num += 1;
    if ((x != 0) && (incol == ba[x - 1][y]) && (point[x - 1][y] == 0))
        saiki_left(ba, point, x - 1, y, num, incol);
    if ((y != 11) && (incol == ba[x][y + 1]) && (point[x][y + 1] == 0))
        saiki_up(ba, point, x, y + 1, num, incol);
    if ((x != 5) && (incol == ba[x + 1][y]) && (point[x + 1][y] == 0))
        saiki_right(ba, point, x + 1, y, num, incol);
}

void saiki_down(const TLColor ba[][kHeight], int point[][12], int x, int y, int* num, TLColor incol)
{
    point[x][y] = 1;
    *num += 1;
    if ((x != 0) && (incol == ba[x - 1][y]) && (point[x - 1][y] == 0))
        saiki_left(ba, point, x - 1, y, num, incol);
    if ((x != 5) && (incol == ba[x + 1][y]) && (point[x + 1][y] == 0))
        saiki_right(ba, point, x + 1, y, num, incol);
    if ((y != 0) && (incol == ba[x][y - 1]) && (point[x][y - 1] == 0))
        saiki_down(ba, point, x, y - 1, num, incol);
}

void saiki_3_right(const TLColor ba[][kHeight], int point[][12], int x, int y, int* num, TLColor incol)
{
    point[x][y] = 1;
    *num += 1;
    if ((y != 11) && (incol == ba[x][y + 1]) && (point[x][y + 1] != 1))
        saiki_3_up(ba, point, x, y + 1, num, incol);
    if ((x != 5) && (incol == ba[x + 1][y]) && (point[x + 1][y] != 1))
        saiki_3_right(ba, point, x + 1, y, num, incol);
    if ((y != 0) && (incol == ba[x][y - 1]) && (point[x][y - 1] != 1))
        saiki_3_down(ba, point, x, y - 1, num, incol);
}

void saiki_3_left(const TLColor ba[][kHeight], int point[][12], int x, int y, int* num, TLColor incol)
{
    point[x][y] = 1;
    *num += 1;
    if ((x != 0) && (incol == ba[x - 1][y]) && (point[x - 1][y] != 1))
        saiki_3_left(ba, point, x - 1, y, num, incol);
    if ((y != 11) && (incol == ba[x][y + 1]) && (point[x][y + 1] != 1))
        saiki_3_up(ba, point, x, y + 1, num, incol);
    if ((y != 0) && (incol == ba[x][y - 1]) && (point[x][y - 1] != 1))
        saiki_3_down(ba, point, x, y - 1, num, incol);
}

void saiki_3_up(const TLColor ba[][kHeight], int point[][12], int x, int y, int* num, TLColor incol)
{
    point[x][y] = 1;
    *num += 1;
    if ((x != 0) && (incol == ba[x - 1][y]) && (point[x - 1][y] != 1))
        saiki_3_left(ba, point, x - 1, y, num, incol);
    if ((y != 11) && (incol == ba[x][y + 1]) && (point[x][y + 1] != 1))
        saiki_3_up(ba, point, x, y + 1, num, incol);
    if ((x != 5) && (incol == ba[x + 1][y]) && (point[x + 1][y] != 1))
        saiki_3_right(ba, point, x + 1, y, num, incol);
}

void saiki_3_down(const TLColor ba[][kHeight], int point[][12], int x, int y, int* num, TLColor incol)
{
    point[x][y] = 1;
    *num += 1;
    if ((x != 0) && (incol == ba[x - 1][y]) && (point[x - 1][y] != 1))
        saiki_3_left(ba, point, x - 1, y, num, incol);
    if ((x != 5) && (incol == ba[x + 1][y]) && (point[x + 1][y] != 1))
        saiki_3_right(ba, point, x + 1, y, num, incol);
    if ((y != 0) && (incol == ba[x][y - 1]) && (point[x][y - 1] != 1))
        saiki_3_down(ba, point, x, y - 1, num, incol);
}

void syou_right(TLColor ba[][kHeight], int x, int y, TLColor incol, int flg[])
{
    ba[x][y] = TLColor::EMPTY;
    flg[x] = 1;
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up(ba, x, y + 1, incol, flg);
    if ((y != 11) && (ba[x][y + 1] == TLColor::OJAMA))
        ba[x][y + 1] = TLColor::EMPTY;
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right(ba, x + 1, y, incol, flg);
    if ((x != 5) && (ba[x + 1][y] == TLColor::OJAMA)) {
        ba[x + 1][y] = TLColor::EMPTY;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down(ba, x, y - 1, incol, flg);
    if ((y != 0) && (ba[x][y - 1] == TLColor::OJAMA))
        ba[x][y - 1] = TLColor::EMPTY;
}

void syou_left(TLColor ba[][kHeight], int x, int y, TLColor incol, int flg[])
{
    ba[x][y] = TLColor::EMPTY;
    flg[x] = 1;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left(ba, x - 1, y, incol, flg);
    if ((x != 0) && (ba[x - 1][y] == TLColor::OJAMA)) {
        ba[x - 1][y] = TLColor::EMPTY;
        flg[x - 1] = 1;
    }
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up(ba, x, y + 1, incol, flg);
    if ((y != 11) && (ba[x][y + 1] == TLColor::OJAMA))
        ba[x][y + 1] = TLColor::EMPTY;
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down(ba, x, y - 1, incol, flg);
    if ((y != 0) && (ba[x][y - 1] == TLColor::OJAMA))
        ba[x][y - 1] = TLColor::EMPTY;
}

void syou_up(TLColor ba[][kHeight], int x, int y, TLColor incol, int flg[])
{
    ba[x][y] = TLColor::EMPTY;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left(ba, x - 1, y, incol, flg);
    if ((x != 0) && (ba[x - 1][y] == TLColor::OJAMA)) {
        ba[x - 1][y] = TLColor::EMPTY;
        flg[x - 1] = 1;
    }
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up(ba, x, y + 1, incol, flg);
    if ((y != 11) && (ba[x][y + 1] == TLColor::OJAMA))
        ba[x][y + 1] = TLColor::EMPTY;
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right(ba, x + 1, y, incol, flg);
    if ((x != 5) && (ba[x + 1][y] == TLColor::OJAMA)) {
        ba[x + 1][y] = TLColor::EMPTY;
        flg[x + 1] = 1;
    }
}

void syou_down(TLColor ba[][kHeight], int x, int y, TLColor incol, int flg[])
{
    ba[x][y] = TLColor::EMPTY;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left(ba, x - 1, y, incol, flg);
    if ((x != 0) && (ba[x - 1][y] == TLColor::OJAMA)) {
        ba[x - 1][y] = TLColor::EMPTY;
        flg[x - 1] = 1;
    }
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right(ba, x + 1, y, incol, flg);
    if ((x != 5) && (ba[x + 1][y] == TLColor::OJAMA)) {
        ba[x + 1][y] = TLColor::EMPTY;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down(ba, x, y - 1, incol, flg);
    if ((y != 0) && (ba[x][y - 1] == TLColor::OJAMA))
        ba[x][y - 1] = TLColor::EMPTY;
}

void syou_right_num(TLColor ba[][kHeight], int x, int y, TLColor incol, int flg[], int* num)
{
    ba[x][y] = TLColor::EMPTY;
    flg[x] = 1;
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up_num(ba, x, y + 1, incol, flg, num);
    if ((y != 11) && (ba[x][y + 1] == TLColor::OJAMA))
        ba[x][y + 1] = TLColor::EMPTY;
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right_num(ba, x + 1, y, incol, flg, num);
    if ((x != 5) && (ba[x + 1][y] == TLColor::OJAMA)) {
        ba[x + 1][y] = TLColor::EMPTY;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down_num(ba, x, y - 1, incol, flg, num);
    if ((y != 0) && (ba[x][y - 1] == TLColor::OJAMA))
        ba[x][y - 1] = TLColor::EMPTY;
}

void syou_left_num(TLColor ba[][kHeight], int x, int y, TLColor incol, int flg[], int* num)
{
    ba[x][y] = TLColor::EMPTY;
    flg[x] = 1;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left_num(ba, x - 1, y, incol, flg, num);
    if ((x != 0) && (ba[x - 1][y] == TLColor::OJAMA)) {
        ba[x - 1][y] = TLColor::EMPTY;
        flg[x - 1] = 1;
    }
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up_num(ba, x, y + 1, incol, flg, num);
    if ((y != 11) && (ba[x][y + 1] == TLColor::OJAMA))
        ba[x][y + 1] = TLColor::EMPTY;
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down_num(ba, x, y - 1, incol, flg, num);
    if ((y != 0) && (ba[x][y - 1] == TLColor::OJAMA))
        ba[x][y - 1] = TLColor::EMPTY;
}

void syou_up_num(TLColor ba[][kHeight], int x, int y, TLColor incol, int flg[], int* num)
{
    ba[x][y] = TLColor::EMPTY;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left_num(ba, x - 1, y, incol, flg, num);
    if ((x != 0) && (ba[x - 1][y] == TLColor::OJAMA)) {
        ba[x - 1][y] = TLColor::EMPTY;
        flg[x - 1] = 1;
    }
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up_num(ba, x, y + 1, incol, flg, num);
    if ((y != 11) && (ba[x][y + 1] == TLColor::OJAMA))
        ba[x][y + 1] = TLColor::EMPTY;
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right_num(ba, x + 1, y, incol, flg, num);
    if ((x != 5) && (ba[x + 1][y] == TLColor::OJAMA)) {
        ba[x + 1][y] = TLColor::EMPTY;
        flg[x + 1] = 1;
    }
}

void syou_down_num(TLColor ba[][kHeight], int x, int y, TLColor incol, int flg[], int* num)
{
    ba[x][y] = TLColor::EMPTY;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left_num(ba, x - 1, y, incol, flg, num);
    if ((x != 0) && (ba[x - 1][y] == TLColor::OJAMA)) {
        ba[x - 1][y] = TLColor::EMPTY;
        flg[x - 1] = 1;
    }
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right_num(ba, x + 1, y, incol, flg, num);
    if ((x != 5) && (ba[x + 1][y] == TLColor::OJAMA)) {
        ba[x + 1][y] = TLColor::EMPTY;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down_num(ba, x, y - 1, incol, flg, num);
    if ((y != 0) && (ba[x][y - 1] == TLColor::OJAMA))
        ba[x][y - 1] = TLColor::EMPTY;
}

} // namespace

bool isTLFieldEmpty(const TLColor field[6][kHeight])
{
    for (int i = 0; i < 6; ++i) {
        if (field[i][0] != TLColor::EMPTY) {
            return false;
        }
    }
    return true;
}

int countNormalColor13(const TLColor f[][kHeight])
{
    int n = 0;
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 13; j++) {
            if (isNormalTLColor(f[i][j]))
                ++n;
        }
    }

    return n;
}

void copyField(const TLColor src[][kHeight], TLColor dst[][kHeight])
{
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < kHeight; ++j) {
            dst[i][j] = src[i][j];
        }
    }
}

TLRensaResult simulate(TLColor field[][kHeight])
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
        int point[6][12] {};
        int rakkaflg[6] {};

        SmallIntSet used_colors;
        // check connections and vanish
        for (int i = 0; i < 6; ++i) {
            for (int j = bottom[i]; j < 12; ++j) {
                TLColor color = field[i][j];
                if (color == TLColor::EMPTY)
                    break;
                if (point[i][j] != 1 && isNormalTLColor(color)) {
                    int num = 0;
                    saiki(field, point, i, j, &num, color);
                    if (num >= 4) {
                        cont = true;
                        syou(field, i, j, color, rakkaflg);

                        long_bonus[chain] += longBonus(num);
                        num_connected[chain] += num;
                        used_colors.set(color);
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
                    if (field[i][j] == TLColor::EMPTY) {
                        if (n == 0)
                            bottom[i] = j;
                        n++;
                    } else if (n != 0) {
                        field[i][j - n] = field[i][j];
                        field[i][j] = TLColor::EMPTY;
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

void saiki(const TLColor ba[][kHeight], int point[][12], int x, int y, int* num, TLColor incol)
{
    DCHECK(isNormalTLColor(incol))
            << "saiki(x=" << x << ",y=" << y << ",incol=" << incol << ")";

    point[x][y] = 1;
    *num += 1;
    if ((x != 0) && (incol == ba[x - 1][y]) && (point[x - 1][y] == 0))
        saiki_left(ba, point, x - 1, y, num, incol);
    if ((y != 11) && (incol == ba[x][y + 1]) && (point[x][y + 1] == 0))
        saiki_up(ba, point, x, y + 1, num, incol);
    if ((x != 5) && (incol == ba[x + 1][y]) && (point[x + 1][y] == 0))
        saiki_right(ba, point, x + 1, y, num, incol);
    if ((y != 0) && (incol == ba[x][y - 1]) && (point[x][y - 1] == 0))
        saiki_down(ba, point, x, y - 1, num, incol);
}

void saiki_3(const TLColor ba[][kHeight], int point[][12], int x, int y, int* num, TLColor incol)
{
    point[x][y] = 1;
    *num += 1;
    if ((x != 0) && (incol == ba[x - 1][y]) && (point[x - 1][y] != 1))
        saiki_3_left(ba, point, x - 1, y, num, incol);
    if ((y != 11) && (incol == ba[x][y + 1]) && (point[x][y + 1] != 1))
        saiki_3_up(ba, point, x, y + 1, num, incol);
    if ((x != 5) && (incol == ba[x + 1][y]) && (point[x + 1][y] != 1))
        saiki_3_right(ba, point, x + 1, y, num, incol);
    if ((y != 0) && (incol == ba[x][y - 1]) && (point[x][y - 1] != 1))
        saiki_3_down(ba, point, x, y - 1, num, incol);
}

void saiki_4(TLColor ba[][kHeight], int x, int y, int* num, TLColor incol)
{
    ba[x][y] = TLColor::EMPTY;
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

void syou(TLColor ba[][kHeight], int x, int y, TLColor incol, int flg[])
{
    ba[x][y] = TLColor::EMPTY;
    flg[x] = 1;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left(ba, x - 1, y, incol, flg);
    if ((x != 0) && (ba[x - 1][y] == TLColor::OJAMA)) {
        ba[x - 1][y] = TLColor::EMPTY;
        flg[x - 1] = 1;
    }
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up(ba, x, y + 1, incol, flg);
    if ((y != 11) && (ba[x][y + 1] == TLColor::OJAMA))
        ba[x][y + 1] = TLColor::EMPTY;
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right(ba, x + 1, y, incol, flg);
    if ((x != 5) && (ba[x + 1][y] == TLColor::OJAMA)) {
        ba[x + 1][y] = TLColor::EMPTY;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down(ba, x, y - 1, incol, flg);
    if ((y != 0) && (ba[x][y - 1] == TLColor::OJAMA))
        ba[x][y - 1] = TLColor::EMPTY;
}

void syou_downx(TLColor ba[][kHeight], int x, int y, TLColor incol, int flg[], int* num)
{
    *num += 1;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left_num(ba, x - 1, y, incol, flg, num);
    if ((x != 0) && (ba[x - 1][y] == TLColor::OJAMA)) {
        ba[x - 1][y] = TLColor::EMPTY;
        flg[x - 1] = 1;
    }
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right_num(ba, x + 1, y, incol, flg, num);
    if ((x != 5) && (ba[x + 1][y] == TLColor::OJAMA)) {
        ba[x + 1][y] = TLColor::EMPTY;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down_num(ba, x, y - 1, incol, flg, num);
    if ((y != 0) && (ba[x][y - 1] == TLColor::OJAMA))
        ba[x][y - 1] = TLColor::EMPTY;
}

bool setti_puyo(TLColor ba[][kHeight], int aa, TLColor nx1, TLColor nx2, int setti_basyo[])
{
    if (aa < 6) {
        setti_basyo[0] = aa;
        setti_basyo[1] = -1;
        setti_basyo[2] = aa;
        setti_basyo[3] = -1;
        for (int j = 0; j < 13; j++) {
            if (ba[aa][j] == TLColor::EMPTY) {
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
            if (ba[aa - 6][j] == TLColor::EMPTY) {
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
            if (ba[aa - 11][j] == TLColor::EMPTY) {
                setti_basyo[1] = j;
                break;
            }
        }
        for (int j = 0; j < 13; j++) {
            if (ba[aa - 12][j] == TLColor::EMPTY) {
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
            if (ba[aa - 17][j] == TLColor::EMPTY) {
                setti_basyo[1] = j;
                break;
            }
        }
        for (int j = 0; j < 13; j++) {
            if (ba[aa - 16][j] == TLColor::EMPTY) {
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

int setti_puyo_1(TLColor ba[][kHeight], int eex, TLColor eecol)
{
    int j;
    int oita = 0;
    int num = 0;
    int setti_basyoy;
    for (j = 0; j < 12; j++) {
        if (ba[eex][j] == TLColor::EMPTY) {
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

int chousei_syoukyo(TLColor ba[][kHeight], int setti_basyo[])
{
    int num = 0;
    int numa = 0;
    int numb = 0;
    int point[6][12] {};
    int i, j;
    int syo = 1;
    int kiept[6] = { 0 };
    int rakkaflg[6] = { 0 };
    int n;
    int a, b, c, d;
    int keshiko = 0;

    a = setti_basyo[0];
    b = setti_basyo[1];
    c = setti_basyo[2];
    d = setti_basyo[3];
    if ((b < 12) && (b >= 0)) {
        saiki(ba, point, a, b, &numa, ba[a][b]);
    }
    if ((d < 12) && (d >= 0)) {
        if (point[c][d] == 0) {
            saiki(ba, point, c, d, &numb, ba[c][d]);
        }
    }
    if ((numa < 4) && (numb < 4))
        return 0;
    if (numa > 3) {
        syou(ba, a, b, ba[a][b], rakkaflg);
        keshiko += numa;
    }
    if (numb > 3) {
        syou(ba, c, d, ba[c][d], rakkaflg);
        keshiko += numb;
    }

    for (i = 0; i < 6; i++) {
        kiept[i] = 12;
        if (rakkaflg[i] == 1) {
            n = 0;
            for (j = 0; j < 13; j++) {
                if (ba[i][j] == TLColor::EMPTY) {
                    if (n == 0)
                        kiept[i] = j;
                    n++;
                } else if (n != 0) {
                    ba[i][j - n] = ba[i][j];
                    ba[i][j] = TLColor::EMPTY;
                }
            }
        }
    }

    while (syo) {
        syo = 0;
        memset(point, 0, sizeof(point));
        rakkaflg[0] = 0;
        rakkaflg[1] = 0;
        rakkaflg[2] = 0;
        rakkaflg[3] = 0;
        rakkaflg[4] = 0;
        rakkaflg[5] = 0;
        for (i = 0; i < 6; i++) {
            for (j = kiept[i]; j < 12; j++) {
                if (point[i][j] != 0)
                    continue;
                if (ba[i][j] == TLColor::EMPTY)
                    break;
                if (ba[i][j] != TLColor::OJAMA) {
                    saiki(ba, point, i, j, &num, ba[i][j]);
                    if (num > 3) {
                        syo = 1;
                        syou(ba, i, j, ba[i][j], rakkaflg);
                        keshiko += num;
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
                    if (ba[i][j] == TLColor::EMPTY) {
                        if (n == 0)
                            kiept[i] = j;
                        n++;
                    } else if (n != 0) {
                        ba[i][j - n] = ba[i][j];
                        ba[i][j] = TLColor::EMPTY;
                    }
                }
            }
        }
    }

    return keshiko;
}

int chousei_syoukyo_2(TLColor ba[][kHeight], int setti_basyo[], int* chain, int dabuchk[], int* ichiren_kesi, int* score)
{
    int rensa_rate[19] = { 0, 8, 16, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480, 512 };
    int color_rate[5] = { 0, 3, 6, 12, 24 };
    int renketsu[19][5] {};
    int colnum;
    int renketsunum;
    int renketsubonus[19] = { 0 };
    int rate;
    int color;

    int num = 0;
    int numa = 0;
    int numb = 0;
    int point[6][12] {};
    int i, j;
    int syo = 1;
    int kiept[6] = { 0 };
    int rakkaflg[6] = { 0 };
    int n;
    int a, b, c, d;
    int keshiko = 0;
    for (i = 0; i < 20; i++) {
        dabuchk[i] = 0;
    }
    (*ichiren_kesi) = 0;
    *score = 0;

    a = setti_basyo[0];
    b = setti_basyo[1];
    c = setti_basyo[2];
    d = setti_basyo[3];
    if ((b < 12) && (b >= 0)) {
        saiki(ba, point, a, b, &numa, ba[a][b]);
    }
    if ((d < 12) && (d >= 0)) {
        if (point[c][d] == 0) {
            saiki(ba, point, c, d, &numb, ba[c][d]);
        }
    }
    (*chain) = 0;
    if ((numa < 4) && (numb < 4))
        return 0;
    if (numa > 3) {
        color = ba[a][b];
        renketsu[*chain][color - 1] += numa;
        if (numa > 10)
            renketsubonus[*chain] += 10; // bugggggg 111102
        else if (numa > 4)
            renketsubonus[*chain] += numa - 3;
        syou(ba, a, b, ba[a][b], rakkaflg);
        keshiko += numa;
    }
    if (numb > 3) {
        color = ba[c][d];
        renketsu[*chain][color - 1] += numb;
        if (numb > 10)
            renketsubonus[*chain] += 10; // bugggggg 111102
        else if (numb > 4)
            renketsubonus[*chain] += numb - 3;
        syou(ba, c, d, ba[c][d], rakkaflg);
        keshiko += numb;
    }

    for (i = 0; i < 6; i++) {
        kiept[i] = 12;
        if (rakkaflg[i] == 1) {
            n = 0;
            for (j = 0; j < 13; j++) {
                if (ba[i][j] == TLColor::EMPTY) {
                    if (n == 0)
                        kiept[i] = j;
                    n++;
                } else if (n != 0) {
                    ba[i][j - n] = ba[i][j];
                    ba[i][j] = TLColor::EMPTY;
                }
            }
        }
    }
    (*chain) = 1;

    while (syo) {
        syo = 0;
        memset(point, 0, sizeof(point));
        rakkaflg[0] = 0;
        rakkaflg[1] = 0;
        rakkaflg[2] = 0;
        rakkaflg[3] = 0;
        rakkaflg[4] = 0;
        rakkaflg[5] = 0;
        for (i = 0; i < 6; i++) {
            for (j = kiept[i]; j < 12; j++) {
                if (ba[i][j] == TLColor::EMPTY)
                    continue;
                if ((point[i][j] != 1) && (ba[i][j] != TLColor::OJAMA)) {
                    saiki(ba, point, i, j, &num, ba[i][j]);
                    if (num > 3) {
                        syo = 1;
                        color = ba[i][j];
                        renketsu[*chain][color - 1] += num;
                        if (num > 10)
                            renketsubonus[*chain] += 10; // bugggggg 111102
                        else if (num > 4)
                            renketsubonus[*chain] += num - 3;
                        syou(ba, i, j, ba[i][j], rakkaflg);
                        keshiko += num;
                        dabuchk[*chain]++;
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
                    if (ba[i][j] == TLColor::EMPTY) {
                        if (n == 0)
                            kiept[i] = j;
                        n++;
                    } else if (n != 0) {
                        ba[i][j - n] = ba[i][j];
                        ba[i][j] = TLColor::EMPTY;
                    }
                }
            }
        }
        if (syo == 1)
            *chain += 1;
    }
    for (i = 0; i < (*chain); i++) {
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
    return keshiko;
}

int chousei_syoukyo_3(TLColor bass[][kHeight], int[], int* poi2s, int* score, int tokus, int i2, int j2, int ruiseki_point)
{
    int rensa_rate[19] = { 0, 8, 16, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480, 512 };
    int color_rate[5] = { 0, 3, 6, 12, 24 };
    int renketsu[19][5] {};
    int colnum;
    int renketsunum;
    int renketsubonus[19] = { 0 };
    int rate;
    int color;

    int num = 0;
    int point[6][12] {};
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
    color = bass[i2][j2];
    if (tokus < 5) {
        syou_downx(bass, i2, j2 + 1, bass[i2][j2], rakkaflg, &num);
    } else if (tokus < 7) {
        syou_downx(bass, i2 + 1, j2, bass[i2][j2], rakkaflg, &num);
    } else if (tokus == 7) {
        syou_downx(bass, i2 - 1, j2, bass[i2][j2], rakkaflg, &num);
    }
    renketsu[0][color - 1] = num;
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
                if (bass[i][j] == 0) {
                    if (n == 0)
                        kiept[i] = j;
                    n++;
                } else if (n != 0) {
                    bass[i][j - n] = bass[i][j];
                    bass[i][j] = TLColor::EMPTY;
                }
            }
            rakka_ruiseki += n;
        }
    }

    while (syo) {
        syo = 0;
        memset(point, 0, sizeof(point));
        rakkaflg[0] = 0;
        rakkaflg[1] = 0;
        rakkaflg[2] = 0;
        rakkaflg[3] = 0;
        rakkaflg[4] = 0;
        rakkaflg[5] = 0;
        for (i = 0; i < 6; i++) {
            for (j = kiept[i]; j < 12; j++) {
                if (point[i][j] != 0)
                    continue;
                if (bass[i][j] == TLColor::EMPTY)
                    break;
                if (bass[i][j] != TLColor::OJAMA) {
                    saiki(bass, point, i, j, &num, bass[i][j]);
                    if (num > 3) {
                        syo = 1;
                        color = bass[i][j];
                        renketsu[chain][color - 1] += num;
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
                    if (bass[i][j] == TLColor::EMPTY) {
                        if (n == 0)
                            kiept[i] = j;
                        n++;
                    } else if (n != 0) {
                        bass[i][j - n] = bass[i][j];
                        bass[i][j] = TLColor::EMPTY;
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

int chousei_syoukyo_sc(TLColor ba[][kHeight], int setti_basyo[], int* score)
{
    int rensa_rate[19] = { 0, 8, 16, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480, 512 };
    int color_rate[5] = { 0, 3, 6, 12, 24 };
    int renketsu[19][5] {};
    int colnum;
    int renketsunum;
    int renketsubonus[19] = { 0 };
    int rate;
    int color;

    int num = 0;
    int numa = 0;
    int numb = 0;
    int point[6][12] {};
    int i, j;
    int syo = 1;
    int kiept[6] = { 0 };
    int rakkaflg[6] = { 0 };
    int n;
    int a, b, c, d;
    int keshiko = 0;
    int chain;
    *score = 0;

    a = setti_basyo[0];
    b = setti_basyo[1];
    c = setti_basyo[2];
    d = setti_basyo[3];
    if ((b < 12) && (b >= 0)) {
        saiki(ba, point, a, b, &numa, ba[a][b]);
    }
    if ((d < 12) && (d >= 0)) {
        if (point[c][d] == 0) {
            saiki(ba, point, c, d, &numb, ba[c][d]);
        }
    }
    (chain) = 0;
    if ((numa < 4) && (numb < 4))
        return 0;
    if (numa > 3) {
        color = ba[a][b];
        renketsu[chain][color - 1] += numa;
        if (numa > 10)
            renketsubonus[chain] += 10; // bugggggg 111102
        else if (numa > 4)
            renketsubonus[chain] += numa - 3;
        syou(ba, a, b, ba[a][b], rakkaflg);
        keshiko += numa;
    }
    if (numb > 3) {
        color = ba[c][d];
        renketsu[chain][color - 1] += numb;
        if (numb > 10)
            renketsubonus[chain] += 10; // bugggggg 111102
        else if (numb > 4)
            renketsubonus[chain] += numb - 3;
        syou(ba, c, d, ba[c][d], rakkaflg);
        keshiko += numb;
    }

    for (i = 0; i < 6; i++) {
        kiept[i] = 12;
        if (rakkaflg[i] == 1) {
            n = 0;
            for (j = 0; j < 13; j++) {
                if (ba[i][j] == TLColor::EMPTY) {
                    if (n == 0)
                        kiept[i] = j;
                    n++;
                } else if (n != 0) {
                    ba[i][j - n] = ba[i][j];
                    ba[i][j] = TLColor::EMPTY;
                }
            }
        }
    }
    (chain) = 1;

    while (syo) {
        syo = 0;
        memset(point, 0, sizeof(point));
        rakkaflg[0] = 0;
        rakkaflg[1] = 0;
        rakkaflg[2] = 0;
        rakkaflg[3] = 0;
        rakkaflg[4] = 0;
        rakkaflg[5] = 0;
        for (i = 0; i < 6; i++) {
            for (j = kiept[i]; j < 12; j++) {
                if (ba[i][j] == TLColor::EMPTY)
                    continue;
                if ((point[i][j] != 1) && (ba[i][j] != TLColor::OJAMA)) {
                    saiki(ba, point, i, j, &num, ba[i][j]);
                    if (num > 3) {
                        syo = 1;
                        color = ba[i][j];
                        renketsu[chain][color - 1] += num;
                        if (num > 10)
                            renketsubonus[chain] += 10; // bugggggg 111102
                        else if (num > 4)
                            renketsubonus[chain] += num - 3;
                        syou(ba, i, j, ba[i][j], rakkaflg);
                        keshiko += num;
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
                    if (ba[i][j] == TLColor::EMPTY) {
                        if (n == 0)
                            kiept[i] = j;
                        n++;
                    } else if (n != 0) {
                        ba[i][j - n] = ba[i][j];
                        ba[i][j] = TLColor::EMPTY;
                    }
                }
            }
        }
        if (syo == 1)
            chain += 1;
    }
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
    return keshiko;
}

int hon_syoukyo(TLColor ba[][kHeight])
{
    int num = 0;
    int point[6][12] {};
    int i, j;
    int syo = 1;
    int kiept[6] = { 0 };
    int rakkaflg[6];
    int n;
    int chain = 0;

    while (syo) {
        syo = 0;
        memset(point, 0, sizeof(point));
        rakkaflg[0] = 0;
        rakkaflg[1] = 0;
        rakkaflg[2] = 0;
        rakkaflg[3] = 0;
        rakkaflg[4] = 0;
        rakkaflg[5] = 0;
        for (i = 0; i < 6; i++) {
            for (j = kiept[i]; j < 12; j++) {
                if (point[i][j] != 0)
                    continue;
                if (ba[i][j] == TLColor::EMPTY)
                    break;
                if (ba[i][j] != TLColor::OJAMA) {
                    saiki(ba, point, i, j, &num, ba[i][j]);
                    if (num > 3) {
                        syo = 1;
                        syou(ba, i, j, ba[i][j], rakkaflg);
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
                    if (ba[i][j] == TLColor::EMPTY) {
                        if (n == 0)
                            kiept[i] = j;
                        n++;
                    } else if (n != 0) {
                        ba[i][j - n] = ba[i][j];
                        ba[i][j] = TLColor::EMPTY;
                    }
                }
            }
        }
        if (syo == 1)
            chain++;
    }
    return chain;
}

int hon_syoukyo_score(TLColor ba[][kHeight], int* score, int* quick)
{
    int rensa_rate[19] = { 0, 8, 16, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480, 512 };
    int color_rate[5] = { 0, 3, 6, 12, 24 };
    int renketsu[19][5] {};
    int colnum;
    int renketsunum;
    int renketsubonus[19] = { 0 };
    int rate;
    int color;

    int num = 0;
    int point[6][12] {};
    int i, j;
    int syo = 1;
    int kiept[6] = { 0 };
    int rakkaflg[6];
    int n;
    int chain = 0;

    (*quick) = 1;

    while (syo) {
        syo = 0;
        memset(point, 0, sizeof(point));
        rakkaflg[0] = 0;
        rakkaflg[1] = 0;
        rakkaflg[2] = 0;
        rakkaflg[3] = 0;
        rakkaflg[4] = 0;
        rakkaflg[5] = 0;
        for (i = 0; i < 6; i++) {
            for (j = kiept[i]; j < 12; j++) {
                if (point[i][j] != 0)
                    continue;
                if (ba[i][j] == TLColor::EMPTY)
                    break;
                if (ba[i][j] != TLColor::OJAMA) {
                    saiki(ba, point, i, j, &num, ba[i][j]);
                    if (num > 3) {
                        syo = 1;
                        color = ba[i][j];
                        renketsu[chain][color - 1] += num;
                        if (num > 10)
                            renketsubonus[chain] += 10; // bugggggg 111102
                        else if (num > 4)
                            renketsubonus[chain] += num - 3;
                        syou(ba, i, j, ba[i][j], rakkaflg);
                    }
                    num = 0;
                }
            }
        }
        if (syo == 1)
            (*quick) = 1;
        for (i = 0; i < 6; i++) {
            kiept[i] = 12;
            if (rakkaflg[i] == 1) {
                n = 0;
                for (j = 0; j < 13; j++) {
                    if (ba[i][j] == TLColor::EMPTY) {
                        if (n == 0)
                            kiept[i] = j;
                        n++;
                    } else if (n != 0) {
                        ba[i][j - n] = ba[i][j];
                        ba[i][j] = TLColor::EMPTY;
                        (*quick) = 0;
                    }
                }
            }
        }
        if (syo == 1)
            chain++;
    }

    *score = 0;
    for (i = 0; i < chain; i++) {
        rate = 0;
        colnum = 0;
        renketsunum = 0;
        for (j = 0; j < 5; j++) {
            colnum += (renketsu[i][j] != 0);
            renketsunum += renketsu[i][j];
        }
        if (colnum > 0)
            rate = color_rate[colnum - 1] + renketsubonus[i] + rensa_rate[i];
        if (rate == 0)
            rate = 1;
        *score += renketsunum * rate * 10;
    }
    return chain;
}

void setti_ojama(TLColor f[][kHeight], int numOjama)
{
    int lines = (numOjama + 3) / 6;

    for (int i = 0; i < 6; i++) {
        int cnt = 0;
        for (int j = 0; j < 13; j++) {
            if (f[i][j] == TLColor::EMPTY) {
                f[i][j] = TLColor::OJAMA;
                cnt++;
            }

            if (cnt == lines)
                break;
        }
    }
}

}  // namespace test_lockit
