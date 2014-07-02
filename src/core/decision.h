#ifndef CORE_DECISION_H_
#define CORE_DECISION_H_

#include <stdio.h>
#include <string>

#include <glog/logging.h>

#include "base/base.h"

class Decision {
public:
    static const Decision& NoInputDecision() {
        static const Decision no_input(-1, -1);
        return no_input;
    }

public:
    Decision() : x(0), r(0) {}
    Decision(int x0, int r0) : x(x0), r(r0) {}

    int axisX() const { return x; }

    int childX() const
    {
        switch (r) {
        case 0:
            return x;
        case 1:
            return x + 1;
        case 2:
            return x;
        case 3:
            return x - 1;
        default:
            CHECK(false) << r;
            return x;
        }
    }

    bool isValid() const
    {
        if (x <= 0 || 6 < x || r < 0 || 4 <= r)
            return false;

        if ((x == 1 && r == 3) || (x == 6 && r == 1))
            return false;

        return true;
    }

    std::string toString() const
    {
        char buf[80];
        sprintf(buf, "(%d, %d)", x, r);
        return buf;
    }

    bool operator==(const Decision& d) const { return x == d.x && r == d.r; }
    bool operator!=(const Decision& d) const { return !(*this == d); }

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

#endif  // CORE_DECISION_H_
