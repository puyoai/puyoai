#include "core/user_event.h"

using namespace std;

UserEvent::UserEvent(const string& s)
{
    for (const char c : s) {
        switch (c) {
        case 'W': wnextAppeared = true; break;
        case 'G': grounded = true; break;
        case 'P': preDecisionRequest = true; break;
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
    r.resize(7);
    r[0] = wnextAppeared        ? 'W' : '-';
    r[1] = grounded             ? 'G' : '-';
    r[2] = preDecisionRequest   ? 'P' : '-';
    r[3] = decisionRequest      ? 'D' : '-';
    r[4] = decisionRequestAgain ? 'A' : '-';
    r[5] = ojamaDropped         ? 'O' : '-';
    r[6] = puyoErased           ? 'E' : '-';
    return r;
}
