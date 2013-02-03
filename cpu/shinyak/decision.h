#ifndef __DECISION_H__
#define __DECISION_H__

#include <stdio.h>
#include <string>

class Decision {
public:
    Decision() : x(0), r(0) {}
    Decision(int x0, int r0) : x(x0), r(r0) {}

    bool isValid() const
    {
        if (x == 1 && r == 3)
            return false;
        if (x == 6 && r == 1)
            return false;
        return (1 <= x && x <= 6 && 0 <= r && r <= 3);
    }

    std::string toString() const
    {
        char buf[80];
        sprintf(buf, "(%d, %d)", x, r);
        return buf;
    }

    friend bool operator<(const Decision& lhs, const Decision& rhs)
    {
        if (lhs.x != rhs.x)
            return lhs.x < rhs.x;
        return lhs.r < rhs.r;
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

#endif  // __DECISION_H__
