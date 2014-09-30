#include "core/decision.h"

#include <sstream>

using namespace std;

string Decision::toString() const
{
    stringstream ss;
    ss << '(' << x << ", " << r << ")";
    return ss.str();
}

string toString(const vector<Decision>& decisions)
{
    bool isFirst = true;
    stringstream ss;
    for (const auto& d : decisions) {
        if (isFirst) {
            isFirst = false;
        } else {
            ss << "-";
        }
        ss << d.toString();
    }

    return ss.str();
}
