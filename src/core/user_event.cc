#include "core/user_event.h"

using namespace std;

UserEvent::UserEvent(const string& s)
{
    for (const char c : s) {
        switch (c) {
        case 'W': wnextAppeared = true; break;
        case 'G': grounded = true; break;
        case 'D': decisionRequest = true; break;
        case 'A': decisionRequestAgain = true; break;
        case 'O': ojamaDropped = true; break;
        case 'E': puyoErased = true; break;
        }
    }
}

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
