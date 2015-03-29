#ifndef CORE_RENSA_RESULT_H_
#define CORE_RENSA_RESULT_H_

#include <string>

struct RensaResult {
    RensaResult() : chains(0), score(0), frames(0), quick(false) {}

    RensaResult(int chains, int score, int frames, bool quick) : chains(chains), score(score), frames(frames), quick(quick) {}

    std::string toString() const;

    friend bool operator==(const RensaResult& lhs, const RensaResult& rhs)
    {
        return lhs.chains == rhs.chains &&
            lhs.score == rhs.score &&
            lhs.frames == rhs.frames &&
            lhs.quick == rhs.quick;
    }

    friend bool operator!=(const RensaResult& lhs, const RensaResult& rhs) { return !(lhs == rhs); }

    int chains;
    int score;
    int frames;
    bool quick;  // Last vanishment does not drop any puyos.
};

class IgnitionRensaResult {
public:
    IgnitionRensaResult() {}
    IgnitionRensaResult(const RensaResult& rensaResult, int framesToIgnite) :
        rensaResult_(rensaResult), framesToIgnite_(framesToIgnite)
    {}

    const RensaResult& rensaResult() const { return rensaResult_; }

    int score() const { return rensaResult_.score; }
    int chains() const { return rensaResult_.chains; }
    int totalFrames() const { return rensaResult_.frames + framesToIgnite(); }
    int framesToIgnite() const { return framesToIgnite_; }

private:
    RensaResult rensaResult_;
    int framesToIgnite_;
};

#endif
