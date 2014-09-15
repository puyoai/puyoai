#ifndef CORE_STATE_H_
#define CORE_STATE_H_

#include <string>

// TODO(mayah): Remove this.
enum State {
    STATE_NONE = 0,
    STATE_YOU_CAN_PLAY = 1 << 0,
    STATE_WNEXT_APPEARED = 1 << 2,
    STATE_YOU_GROUNDED = 1 << 4,
    STATE_DECISION_REQUEST = 1 << 6,
    STATE_CHAIN_DONE = 1 << 8,
    STATE_OJAMA_DROPPED = 1 << 10,
};

inline std::string GetStateString(int st)
{
    std::string r;
    r.resize(6);
    r[0] = ((st & STATE_YOU_CAN_PLAY) != 0) ? 'P' : '-';
    r[1] = ((st & STATE_WNEXT_APPEARED) != 0) ? 'W' : '-';
    r[2] = ((st & STATE_YOU_GROUNDED) != 0) ? 'G' : '-';
    r[3] = ((st & STATE_DECISION_REQUEST) != 0) ? 'D' : '-';
    r[4] = ((st & STATE_CHAIN_DONE) != 0) ? 'C' : '-';
    r[5] = ((st & STATE_OJAMA_DROPPED) != 0) ? 'O' : '-';
    return r;
}

// TODO(mayah): Use UserState instead of State.
struct UserState {
    void clearEventStates()
    {
        wnextAppeared = false;
        grounded = false;
        decisionRequest = false;
        chainFinished = false;
        ojamaDropped = false;
    }

    bool hasEventState() const
    {
        return wnextAppeared || grounded || decisionRequest || chainFinished || ojamaDropped;
    }

    void clear()
    {
        *this = UserState();
    }

    std::string toString() const
    {
        return GetStateString(toDeprecatedState());
    }

    int toDeprecatedState() const
    {
        int s = 0;
        s |= playable ? STATE_YOU_CAN_PLAY : 0;
        s |= wnextAppeared ? STATE_WNEXT_APPEARED : 0;
        s |= grounded ? STATE_YOU_GROUNDED : 0;
        s |= decisionRequest ? STATE_DECISION_REQUEST : 0;
        s |= chainFinished ? STATE_CHAIN_DONE : 0;
        s |= ojamaDropped ? STATE_OJAMA_DROPPED : 0;

        return s;
    }

    void parseFromDeprecatedState(int state)
    {
        playable        = state & STATE_YOU_CAN_PLAY;
        wnextAppeared   = state & STATE_WNEXT_APPEARED;
        grounded        = state & STATE_YOU_GROUNDED;
        decisionRequest = state & STATE_DECISION_REQUEST;
        chainFinished   = state & STATE_CHAIN_DONE;
        ojamaDropped    = state & STATE_OJAMA_DROPPED;
    }

    // ----- usual states
    bool playable = false;

    // ----- event states
    bool wnextAppeared = false;
    bool grounded = false;
    bool decisionRequest = false;
    bool chainFinished = false;
    bool ojamaDropped = false;
};

#endif  // CORE_STATE_H_
