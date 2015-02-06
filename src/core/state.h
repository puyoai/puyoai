#ifndef CORE_STATE_H_
#define CORE_STATE_H_

#include <string>

// TODO(mayah): Rename to UserEvent.
struct UserState {
    void clear()
    {
        *this = UserState();
    }

    bool hasEventState() const
    {
        return wnextAppeared || grounded || decisionRequest || decisionRequestAgain || chainFinished || ojamaDropped || puyoErased;
    }

    std::string toString() const;

    bool wnextAppeared = false;
    bool grounded = false;
    bool decisionRequest = false;
    bool decisionRequestAgain = false;
    bool chainFinished = false;
    bool ojamaDropped = false;
    bool puyoErased = false;
};

#endif  // CORE_STATE_H_
