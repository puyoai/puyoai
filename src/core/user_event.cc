#include "core/user_event.h"

using namespace std;

string UserEvent::toString() const
{
    std::string r;
    r.resize(6);
    r[0] = wnextAppeared        ? 'W' : '-';
    r[1] = grounded             ? 'G' : '-';
    r[2] = decisionRequest      ? 'D' : '-';
    r[3] = decisionRequestAgain ? 'A' : '-';
    r[4] = ojamaDropped         ? 'O' : '-';
    r[5] = puyoErased           ? 'E' : '-';
    return r;
}
