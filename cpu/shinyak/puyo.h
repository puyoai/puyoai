#ifndef __PUYO_H_
#define __PUYO_H_

#include <glog/logging.h>
#include <string>
#include <vector>
#include "core/puyo.h"
#include "util.h"

// TODO(mayah): Use charOfPuyoColor directly.
inline char toChar(PuyoColor c) {
    return charOfPuyoColor(c);
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
