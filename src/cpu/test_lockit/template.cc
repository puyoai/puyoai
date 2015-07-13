#include "template.h"

#include "color.h"

namespace test_lockit {

int gtr(const int f[][kHeight])
{
    int sc = 0;

    if ((f[0][0] != TLColor::EMPTY) && (f[1][0] != TLColor::EMPTY)) {
        if (f[0][0] == f[1][0]) {
            sc += 1000;
        } else
            sc -= 1000;
    }
    if ((f[0][0] != TLColor::EMPTY) && (f[1][2] != TLColor::EMPTY)) {
        if (f[0][0] == f[1][2]) {
            sc += 1000;
        } else
            sc -= 1000;
    }
    if ((f[0][0] != TLColor::EMPTY) && (f[2][1] != TLColor::EMPTY)) {
        if (f[0][0] == f[2][1]) {
            sc += 1000;
        } else
            sc -= 1000;
    }
    if ((f[1][0] != TLColor::EMPTY) && (f[1][2] != TLColor::EMPTY)) {
        if (f[1][0] == f[1][2]) {
            sc += 1000;
        } else
            sc -= 1000;
    }
    if ((f[1][0] != TLColor::EMPTY) && (f[2][1] != TLColor::EMPTY)) {
        if (f[1][0] == f[2][1]) {
            sc += 1000;
        } else
            sc -= 1000;
    }
    if ((f[2][0] != TLColor::EMPTY) && (f[2][1] != TLColor::EMPTY)) {
        if (f[1][2] == f[2][1]) {
            sc += 1000;
        } else
            sc -= 1000;
    }

    if ((f[0][1] != TLColor::EMPTY) && (f[0][2] != TLColor::EMPTY)) {
        if (f[0][1] == f[0][2]) {
            sc += 1000;
        } else
            sc -= 1000;
    }
    if ((f[0][1] != TLColor::EMPTY) && (f[1][1] != TLColor::EMPTY)) {
        if (f[0][1] == f[1][1]) {
            sc += 1000;
        } else
            sc -= 1000;
    }
    if ((f[1][1] != TLColor::EMPTY) && (f[0][2] != TLColor::EMPTY)) {
        if (f[1][1] == f[0][2]) {
            sc += 1000;
        } else
            sc -= 1000;
    }

    if ((f[0][2] != TLColor::EMPTY) && (f[1][3] != TLColor::EMPTY)) {
        if (f[0][2] == f[0][3]) {
            sc -= 2000;
        }
    }

    if ((f[1][2] != TLColor::EMPTY) && (f[1][3] != TLColor::EMPTY)) {
        if (f[1][2] == f[1][3]) {
            sc -= 500;
        }
    }
    if ((f[1][2] != TLColor::EMPTY) && (f[2][2] != TLColor::EMPTY)) {
        if (f[1][2] == f[2][2]) {
            sc -= 500;
        }
    }
    if ((f[1][0] != TLColor::EMPTY) && (f[2][0] != TLColor::EMPTY)) {
        if (f[1][0] == f[2][0]) {
            sc -= 1000;
        }
    }
    if ((f[2][1] != TLColor::EMPTY) && (f[2][0] != TLColor::EMPTY)) {
        if (f[2][1] == f[2][0]) {
            sc -= 1000;
        }
    }
    if ((f[2][1] != TLColor::EMPTY) && (f[2][2] != TLColor::EMPTY)) {
        if (f[2][1] == f[2][2]) {
            sc -= 500;
        }
    }

    return sc;
}

}  // namespace test_lockit
