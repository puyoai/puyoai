#ifndef CORE_RENSA_RESULT_H_
#define CORE_RENSA_RESULT_H_

#include <ostream>
#include <string>

// RensaResult includes the result of rensa.
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
    friend std::ostream& operator<<(std::ostream& os, const RensaResult& rensaResult) {
        os << "chiains=" << rensaResult.chains << ' '
           << "score=" << rensaResult.score << ' '
           << "frames=" << rensaResult.frames << ' '
           << "quick=" << rensaResult.quick;
        return os;
    }

    int chains;
    int score;
    int frames;
    bool quick; // True if the last vanishment does not drop any puyos.
};

struct RensaStepResult {
    RensaStepResult() {}
    RensaStepResult(int score, int frames, bool quick) : score(score), frames(frames), quick(quick) {}

    int score = 0;
    int frames = 0;
    bool quick = false;
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
    int framesToIgnite() const { return framesToIgnite_; }
    int rensaFrames() const { return rensaResult_.frames; }

    int totalFrames() const { return rensaResult_.frames + framesToIgnite(); }

private:
    RensaResult rensaResult_;
    int framesToIgnite_;
};

#endif
