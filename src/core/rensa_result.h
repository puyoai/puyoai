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

#endif
