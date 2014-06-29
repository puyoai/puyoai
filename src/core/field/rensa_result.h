#ifndef CORE_RENSA_RESULT_H_
#define CORE_RENSA_RESULT_H_

#include <stdint.h>
#include <string>

#include "base/base.h"
#include "core/field/core_field.h"

struct RensaResult {
    RensaResult() : chains(0), score(0), frames(0) {}

    RensaResult(int chains, int score, int frames) : chains(chains), score(score), frames(frames) {}

    std::string toString() const;

    friend bool operator==(const RensaResult& lhs, const RensaResult& rhs) {
        return lhs.chains == rhs.chains &&
            lhs.score == rhs.score &&
            lhs.frames == rhs.frames;
    }

    friend bool operator!=(const RensaResult& lhs, const RensaResult& rhs) {
        return !(lhs == rhs);
    }

    int chains;
    int score;
    int frames;
};

// RensaTrackResult represents in what-th chain puyo is erased.
class RensaTrackResult {
public:
    RensaTrackResult& operator=(const RensaTrackResult& result);

    // Nth Rensa where (x, y) is erased. 0 if not erased.
    int erasedAt(int x, int y) const { return m_erasedAt[x][y]; }
    void setErasedAt(int x, int y, int nthChain) { m_erasedAt[x][y] = nthChain; }

    std::string toString() const;

private:
    uint8_t m_erasedAt[PlainField::MAP_WIDTH][PlainField::MAP_HEIGHT];
};

#endif
