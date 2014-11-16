#ifndef CORE_DECISION_H_
#define CORE_DECISION_H_

#include <string>
#include <vector>

#include "base/base.h"

class Decision {
public:
    constexpr Decision() : x(0), r(0) {}
    constexpr Decision(int x0, int r0) : x(x0), r(r0) {}

    constexpr int axisX() const { return x; }
    constexpr int childX() const { return x + (r == 1) - (r == 3); }
    constexpr int rot() const { return r; }

    bool isValid() const
    {
        if (x <= 0 || 6 < x || r < 0 || 4 <= r)
            return false;

        if ((x == 1 && r == 3) || (x == 6 && r == 1))
            return false;

        return true;
    }

    constexpr Decision reverse() const
    {
        return Decision(childX(), (r + 2) & 0x3);
    }

    std::string toString() const;

    friend bool operator==(const Decision& lhs, const Decision& rhs) { return lhs.x == rhs.x && lhs.r == rhs.r; }
    friend bool operator!=(const Decision& lhs, const Decision& rhs) { return !(lhs == rhs); }

    friend bool operator<(const Decision& lhs, const Decision& rhs)
    {
        return std::make_pair(lhs.x, lhs.r) < std::make_pair(rhs.x, rhs.r);
    }

public:
    // X of the JIKU-PUYO. (1<=x<=6)
    int x;

    // JIKU-PUYO=X KO-PUYO=Y
    // 0:  1:    2:  3:
    //  Y   X Y   X   Y X
    //  X         Y
    int r;
};

std::string toString(const std::vector<Decision>&);

#endif  // CORE_DECISION_H_
