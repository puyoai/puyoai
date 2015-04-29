#include "core/user_event.h"

using namespace std;

string UserEvent::toString() const
{
    std::string r;
    r.resize(7);
    r[0] = wnextAppeared        ? 'W' : '-';
    r[1] = grounded             ? 'G' : '-';
    r[2] = decisionRequest      ? 'D' : '-';
    r[3] = decisionRequestAgain ? 'A' : '-';
    r[4] = chainFinished        ? 'C' : '-';
    r[5] = ojamaDropped         ? 'O' : '-';
    r[6] = puyoErased           ? 'E' : '-';
    return r;
}
