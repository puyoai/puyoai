#ifndef __RENSA_RESULT_H_
#define __RENSA_RESULT_H_

#include <string>
#include "field.h"
#include "util.h"

struct BasicRensaResult {
    BasicRensaResult() 
        : chains(0), score(0), frames(0) {}
    BasicRensaResult(int chains, int score, int frames)
        : chains(chains), score(score), frames(frames) {}

    std::string toString() const;

    int chains;
    int score;
    int frames;
};

class RensaTrackResult {
public:
    RensaTrackResult& operator=(const RensaTrackResult& result);

    // Nth Rensa where (x, y) is erased. 0 if not erased.
    int erasedAt(int x, int y) const { return m_erasedAt[x][y]; }
    void setErasedAt(int x, int y, int nthChain) { m_erasedAt[x][y] = nthChain; }

    std::string toString() const;

private:
    byte m_erasedAt[Field::MAP_WIDTH][Field::MAP_HEIGHT];
};

#endif
