#ifndef CORE_KUMIPUYO_H_
#define CORE_KUMIPUYO_H_

#include <string>

#include "core/puyo_color.h"

class Kumipuyo {
public:
    constexpr Kumipuyo() : axis(PuyoColor::EMPTY), child(PuyoColor::EMPTY) {}
    constexpr Kumipuyo(PuyoColor axis, PuyoColor child) : axis(axis), child(child) {}

    std::string toString() const;

    bool isValid() const { return isNormalColor(axis) && isNormalColor(child); }

    friend bool operator==(const Kumipuyo& lhs, const Kumipuyo& rhs) { return lhs.axis == rhs.axis && lhs.child == rhs.child; }
    friend bool operator!=(const Kumipuyo& lhs, const Kumipuyo& rhs) { return !(lhs == rhs); }

public:
    PuyoColor axis;
    PuyoColor child;
};

#endif
