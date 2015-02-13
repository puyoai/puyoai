#ifndef CORE_RENSA_RESULT_H_
#define CORE_RENSA_RESULT_H_

#include <stdint.h>
#include <string>

#include "base/base.h"
#include "core/field_constant.h"

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

// RensaTrackResult represents in what-th chain puyo is erased.
class RensaTrackResult {
public:
    RensaTrackResult();
    explicit RensaTrackResult(const std::string&);

    RensaTrackResult& operator=(const RensaTrackResult& result);

    // Nth Rensa where (x, y) is erased. 0 if not erased.
    int erasedAt(int x, int y) const { return erasedAt_[x][y]; }
    void setErasedAt(int x, int y, int nthChain) { erasedAt_[x][y] = nthChain; }

    std::string toString() const;

private:
    uint8_t erasedAt_[FieldConstant::MAP_WIDTH][FieldConstant::MAP_HEIGHT];
};

// RensaCoefResult represents
//  - the number of erased puyo in each chain
//  - the coefficient in each chain.
class RensaCoefResult {
public:
    RensaCoefResult() : numErased_{}, coef_{} {}

    void setCoef(int nth, int numErased, int coef)
    {
        numErased_[nth] = numErased;
        coef_[nth] = coef;
    }

    int numErased(int nth) const { return numErased_[nth]; }
    int coef(int nth) const { return coef_[nth]; }

private:
    int numErased_[20]; // numErased does not contain ojama puyos.
    int coef_[20];
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
