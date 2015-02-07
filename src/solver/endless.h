#ifndef SOLVER_ENDLESS_H_
#define SOLVER_ENDLESS_H_

#include <memory>
#include <vector>

#include "core/client/ai/ai.h"
#include "core/decision.h"

struct FrameRequest;
class KumipuyoSeq;

struct EndlessResult {
    int hand;
    int score;
    int maxRensa;
    bool zenkeshi;
    std::vector<Decision> decisions;
};

// Endless implements endless mode. This can be used to check your AI's strength.
class Endless {
public:
    explicit Endless(std::unique_ptr<AI> ai);

    EndlessResult run(const KumipuyoSeq&);

    void setVerbose(bool flag) { verbose_ = flag; }

private:
    void setEnemyField(FrameRequest* req);

    std::unique_ptr<AI> ai_;
    bool verbose_ = false;
};

#endif
