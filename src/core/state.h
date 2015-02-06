#ifndef CORE_STATE_H_
#define CORE_STATE_H_

#include <string>

// TODO(mayah): Remove this.
enum State {
    STATE_NONE = 0,
    // STATE_YOU_CAN_PLAY = 1 << 0,
    STATE_WNEXT_APPEARED = 1 << 2,
    STATE_YOU_GROUNDED = 1 << 4,
    STATE_DECISION_REQUEST = 1 << 6,
    STATE_CHAIN_DONE = 1 << 8,
    STATE_OJAMA_DROPPED = 1 << 10,
    STATE_DECISION_REQUEST_AGAIN = 1 << 12,
    STATE_PUYO_ERASED = 1 << 14,
};

// TODO(mayah): Use UserState instead of State.
// TODO(mayah): Rename to UserEvent.
struct UserState {
    void clearEventStates()
    {
        wnextAppeared = false;
        grounded = false;
        decisionRequest = false;
        decisionRequestAgain = false;
        chainFinished = false;
        ojamaDropped = false;
        puyoErased = false;
    }

    bool hasEventState() const
    {
        return wnextAppeared || grounded || decisionRequest || decisionRequestAgain || chainFinished || ojamaDropped || puyoErased;
    }

    void clear()
    {
        *this = UserState();
    }

    std::string toString() const
    {
        std::string r;
        r.resize(7);
        r[0] = wnextAppeared        ? 'W' : '-';
        r[1] = grounded             ? 'G' : '-';
        r[2] = decisionRequest      ? 'D' : '-';
        r[3] = decisionRequestAgain ? 'C' : '-';
        r[4] = chainFinished        ? 'O' : '-';
        r[5] = ojamaDropped         ? 'A' : '-';
        r[6] = puyoErased           ? 'E' : '-';
        return r;
    }

    int toDeprecatedState() const
    {
        int s = 0;
        s |= wnextAppeared        ? STATE_WNEXT_APPEARED         : 0;
        s |= grounded             ? STATE_YOU_GROUNDED           : 0;
        s |= decisionRequest      ? STATE_DECISION_REQUEST       : 0;
        s |= decisionRequestAgain ? STATE_DECISION_REQUEST_AGAIN : 0;
        s |= chainFinished        ? STATE_CHAIN_DONE             : 0;
        s |= ojamaDropped         ? STATE_OJAMA_DROPPED          : 0;
        s |= puyoErased           ? STATE_PUYO_ERASED            : 0;
        return s;
    }

    void parseFromDeprecatedState(int state)
    {
        wnextAppeared        = state & STATE_WNEXT_APPEARED;
        grounded             = state & STATE_YOU_GROUNDED;
        decisionRequest      = state & STATE_DECISION_REQUEST;
        decisionRequestAgain = state & STATE_DECISION_REQUEST_AGAIN;
        chainFinished        = state & STATE_CHAIN_DONE;
        ojamaDropped         = state & STATE_OJAMA_DROPPED;
        puyoErased           = state & STATE_PUYO_ERASED;
    }

    // ----- event states
    bool wnextAppeared = false;
    bool grounded = false;
    bool decisionRequest = false;
    bool decisionRequestAgain = false;
    bool chainFinished = false;
    bool ojamaDropped = false;
    bool puyoErased = false;
};

#endif  // CORE_STATE_H_
