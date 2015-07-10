#include "field.h"

#include "color.h"

namespace {

void saiki_right(const int[][test_lockit::kHeight], int[][12], int, int, int*, int);
void saiki_left(const int[][test_lockit::kHeight], int[][12], int, int, int*, int);
void saiki_up(const int[][test_lockit::kHeight], int[][12], int, int, int*, int);
void saiki_down(const int[][test_lockit::kHeight], int[][12], int, int, int*, int);

void saiki_3_right(const int[][test_lockit::kHeight], int[][12], int, int, int*, int);
void saiki_3_left(const int[][test_lockit::kHeight], int[][12], int, int, int*, int);
void saiki_3_up(const int[][test_lockit::kHeight], int[][12], int, int, int*, int);
void saiki_3_down(const int[][test_lockit::kHeight], int[][12], int, int, int*, int);

void syou_right(int[][test_lockit::kHeight], int, int, int, int[]);
void syou_left(int[][test_lockit::kHeight], int, int, int, int[]);
void syou_up(int[][test_lockit::kHeight], int, int, int, int[]);
void syou_down(int[][test_lockit::kHeight], int, int, int, int[]);

void syou_right_num(int[][test_lockit::kHeight], int, int, int, int[], int*);
void syou_left_num(int[][test_lockit::kHeight], int, int, int, int[], int*);
void syou_up_num(int[][test_lockit::kHeight], int, int, int, int[], int*);
void syou_down_num(int[][test_lockit::kHeight], int, int, int, int[], int*);

void saiki_right(const int ba[][test_lockit::kHeight], int point[][12], int x, int y, int* num, int incol)
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

void saiki_left(const int ba[][test_lockit::kHeight], int point[][12], int x, int y, int* num, int incol)
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

void saiki_up(const int ba[][test_lockit::kHeight], int point[][12], int x, int y, int* num, int incol)
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

void saiki_down(const int ba[][test_lockit::kHeight], int point[][12], int x, int y, int* num, int incol)
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

void saiki_3_right(const int ba[][test_lockit::kHeight], int point[][12], int x, int y, int* num, int incol)
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

void saiki_3_left(const int ba[][test_lockit::kHeight], int point[][12], int x, int y, int* num, int incol)
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

void saiki_3_up(const int ba[][test_lockit::kHeight], int point[][12], int x, int y, int* num, int incol)
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

void saiki_3_down(const int ba[][test_lockit::kHeight], int point[][12], int x, int y, int* num, int incol)
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

void syou_right(int ba[][test_lockit::kHeight], int x, int y, int incol, int flg[])
{
    ba[x][y] = 0;
    flg[x] = 1;
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up(ba, x, y + 1, incol, flg);
    if ((y != 11) && (ba[x][y + 1] == 6))
        ba[x][y + 1] = 0;
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right(ba, x + 1, y, incol, flg);
    if ((x != 5) && (ba[x + 1][y] == 6)) {
        ba[x + 1][y] = 0;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down(ba, x, y - 1, incol, flg);
    if ((y != 0) && (ba[x][y - 1] == 6))
        ba[x][y - 1] = 0;
}

void syou_left(int ba[][test_lockit::kHeight], int x, int y, int incol, int flg[])
{
    ba[x][y] = 0;
    flg[x] = 1;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left(ba, x - 1, y, incol, flg);
    if ((x != 0) && (ba[x - 1][y] == 6)) {
        ba[x - 1][y] = 0;
        flg[x - 1] = 1;
    }
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up(ba, x, y + 1, incol, flg);
    if ((y != 11) && (ba[x][y + 1] == 6))
        ba[x][y + 1] = 0;
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down(ba, x, y - 1, incol, flg);
    if ((y != 0) && (ba[x][y - 1] == 6))
        ba[x][y - 1] = 0;
}

void syou_up(int ba[][test_lockit::kHeight], int x, int y, int incol, int flg[])
{
    ba[x][y] = 0;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left(ba, x - 1, y, incol, flg);
    if ((x != 0) && (ba[x - 1][y] == 6)) {
        ba[x - 1][y] = 0;
        flg[x - 1] = 1;
    }
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up(ba, x, y + 1, incol, flg);
    if ((y != 11) && (ba[x][y + 1] == 6))
        ba[x][y + 1] = 0;
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right(ba, x + 1, y, incol, flg);
    if ((x != 5) && (ba[x + 1][y] == 6)) {
        ba[x + 1][y] = 0;
        flg[x + 1] = 1;
    }
}

void syou_down(int ba[][test_lockit::kHeight], int x, int y, int incol, int flg[])
{
    ba[x][y] = 0;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left(ba, x - 1, y, incol, flg);
    if ((x != 0) && (ba[x - 1][y] == 6)) {
        ba[x - 1][y] = 0;
        flg[x - 1] = 1;
    }
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right(ba, x + 1, y, incol, flg);
    if ((x != 5) && (ba[x + 1][y] == 6)) {
        ba[x + 1][y] = 0;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down(ba, x, y - 1, incol, flg);
    if ((y != 0) && (ba[x][y - 1] == 6))
        ba[x][y - 1] = 0;
}

void syou_right_num(int ba[][test_lockit::kHeight], int x, int y, int incol, int flg[], int* num)
{
    ba[x][y] = 0;
    flg[x] = 1;
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up_num(ba, x, y + 1, incol, flg, num);
    if ((y != 11) && (ba[x][y + 1] == 6))
        ba[x][y + 1] = 0;
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right_num(ba, x + 1, y, incol, flg, num);
    if ((x != 5) && (ba[x + 1][y] == 6)) {
        ba[x + 1][y] = 0;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down_num(ba, x, y - 1, incol, flg, num);
    if ((y != 0) && (ba[x][y - 1] == 6))
        ba[x][y - 1] = 0;
}

void syou_left_num(int ba[][test_lockit::kHeight], int x, int y, int incol, int flg[], int* num)
{
    ba[x][y] = 0;
    flg[x] = 1;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left_num(ba, x - 1, y, incol, flg, num);
    if ((x != 0) && (ba[x - 1][y] == 6)) {
        ba[x - 1][y] = 0;
        flg[x - 1] = 1;
    }
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up_num(ba, x, y + 1, incol, flg, num);
    if ((y != 11) && (ba[x][y + 1] == 6))
        ba[x][y + 1] = 0;
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down_num(ba, x, y - 1, incol, flg, num);
    if ((y != 0) && (ba[x][y - 1] == 6))
        ba[x][y - 1] = 0;
}

void syou_up_num(int ba[][test_lockit::kHeight], int x, int y, int incol, int flg[], int* num)
{
    ba[x][y] = 0;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left_num(ba, x - 1, y, incol, flg, num);
    if ((x != 0) && (ba[x - 1][y] == 6)) {
        ba[x - 1][y] = 0;
        flg[x - 1] = 1;
    }
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up_num(ba, x, y + 1, incol, flg, num);
    if ((y != 11) && (ba[x][y + 1] == 6))
        ba[x][y + 1] = 0;
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right_num(ba, x + 1, y, incol, flg, num);
    if ((x != 5) && (ba[x + 1][y] == 6)) {
        ba[x + 1][y] = 0;
        flg[x + 1] = 1;
    }
}

void syou_down_num(int ba[][test_lockit::kHeight], int x, int y, int incol, int flg[], int* num)
{
    ba[x][y] = 0;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left_num(ba, x - 1, y, incol, flg, num);
    if ((x != 0) && (ba[x - 1][y] == 6)) {
        ba[x - 1][y] = 0;
        flg[x - 1] = 1;
    }
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right_num(ba, x + 1, y, incol, flg, num);
    if ((x != 5) && (ba[x + 1][y] == 6)) {
        ba[x + 1][y] = 0;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down_num(ba, x, y - 1, incol, flg, num);
    if ((y != 0) && (ba[x][y - 1] == 6))
        ba[x][y - 1] = 0;
}

} // namespace anonymous

namespace test_lockit {

bool IsTLFieldEmpty(const int field[6][kHeight])
{
    for (int i = 0; i < 6; ++i) {
        if (field[i][0] != TLColor::EMPTY) {
            return false;
        }
    }
    return true;
}

void saiki(const int ba[][kHeight], int point[][12], int x, int y, int* num, int incol)
{
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

void saiki_3(const int ba[][kHeight], int point[][12], int x, int y, int* num, int incol)
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

void saiki_4(int ba[][kHeight], int x, int y, int* num, int incol)
{
    ba[x][y] = 0;
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

void syou(int ba[][kHeight], int x, int y, int incol, int flg[])
{
    ba[x][y] = 0;
    flg[x] = 1;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left(ba, x - 1, y, incol, flg);
    if ((x != 0) && (ba[x - 1][y] == 6)) {
        ba[x - 1][y] = 0;
        flg[x - 1] = 1;
    }
    if ((y != 11) && (ba[x][y + 1] == incol))
        syou_up(ba, x, y + 1, incol, flg);
    if ((y != 11) && (ba[x][y + 1] == 6))
        ba[x][y + 1] = 0;
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right(ba, x + 1, y, incol, flg);
    if ((x != 5) && (ba[x + 1][y] == 6)) {
        ba[x + 1][y] = 0;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down(ba, x, y - 1, incol, flg);
    if ((y != 0) && (ba[x][y - 1] == 6))
        ba[x][y - 1] = 0;
}

void syou_downx(int ba[][kHeight], int x, int y, int incol, int flg[], int* num)
{
    *num += 1;
    if ((x != 0) && (ba[x - 1][y] == incol))
        syou_left_num(ba, x - 1, y, incol, flg, num);
    if ((x != 0) && (ba[x - 1][y] == 6)) {
        ba[x - 1][y] = 0;
        flg[x - 1] = 1;
    }
    if ((x != 5) && (ba[x + 1][y] == incol))
        syou_right_num(ba, x + 1, y, incol, flg, num);
    if ((x != 5) && (ba[x + 1][y] == 6)) {
        ba[x + 1][y] = 0;
        flg[x + 1] = 1;
    }
    if ((y != 0) && (ba[x][y - 1] == incol))
        syou_down_num(ba, x, y - 1, incol, flg, num);
    if ((y != 0) && (ba[x][y - 1] == 6))
        ba[x][y - 1] = 0;
}

bool setti_puyo(int ba[][kHeight], int aa, int nx1, int nx2, int setti_basyo[])
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

int setti_puyo_1(int ba[][kHeight], int eex, int eecol)
{
    int j;
    int oita = 0;
    int num = 0;
    int setti_basyoy;
    for (j = 0; j < 12; j++) {
        if (ba[eex][j] == 0) {
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

}  // namespace test_lockit
