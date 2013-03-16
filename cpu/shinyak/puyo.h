#ifndef __PUYO_H_
#define __PUYO_H_

#include <glog/logging.h>
#include <string>
#include <vector>
#include "core/puyo.h"
#include "util.h"

const int NUM_NORMAL_PUYO_COLORS = 4;
inline PuyoColor normalPuyoColorOf(int index) { return static_cast<PuyoColor>(index + 4); }

inline PuyoColor puyoColorOf(char c)
{
    switch (c) {
    case '0':
    case ' ':
        return EMPTY;
    case '1':
    case 'O': // not zero
        return OJAMA;
    case '2':
        return WALL;
    case '4':
    case 'R':
    case 'r':
        return RED;
    case '5':
    case 'B':
    case 'b':
        return BLUE;
    case '6':
    case 'Y':
    case 'y':
        return YELLOW;
    case '7':
    case 'G':
    case 'g':
        return GREEN;
    default:
        return EMPTY;
    }
}

inline char toChar(PuyoColor c)
{
    switch (c) {
    case EMPTY: return ' ';
    case OJAMA: return '@';
    case WALL: return '#';
    case RED: return 'R';
    case BLUE: return 'B';
    case YELLOW: return 'Y';
    case GREEN: return 'G';
    }

    return '?';
}

inline bool isColorPuyo(PuyoColor color)
{
    return RED <= color && color <= GREEN;
}

class KumiPuyo {
public:
    KumiPuyo() : axis(EMPTY), child(EMPTY) {}
    KumiPuyo(PuyoColor axis, PuyoColor child) : axis(axis), child(child) {}

    std::string toString() const {
        char tmp[] = "  ";
        tmp[0] = toChar(axis);
        tmp[1] = toChar(child);

        return tmp;
    }

public:
    PuyoColor axis;
    PuyoColor child;
};

inline void setKumiPuyo(const std::string& str, std::vector<KumiPuyo>& kumiPuyos)
{
    kumiPuyos.clear();

    for (std::string::size_type i = 0; i * 2 + 1 < str.size(); ++i)
        kumiPuyos.push_back(KumiPuyo(puyoColorOf(str[i * 2]), puyoColorOf(str[i * 2 + 1])));
}

#endif
