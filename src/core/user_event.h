#ifndef CORE_USER_EVENT_H_
#define CORE_USER_EVENT_H_

#include <string>

// UserEvent is a set of events in user's field.
struct UserEvent {
    void clear()
    {
        *this = UserEvent();
    }

    bool hasEventState() const
    {
        return wnextAppeared || grounded || decisionRequest || decisionRequestAgain || ojamaDropped || puyoErased;
    }

    std::string toString() const;

    bool wnextAppeared = false;
    bool grounded = false;
    bool decisionRequest = false;
    bool decisionRequestAgain = false;
    bool ojamaDropped = false;
    bool puyoErased = false;
};

#endif // CORE_USER_EVENT_H_
