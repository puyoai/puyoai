#ifndef CORE_CLIENT_AI_ENDLESS_ENDLESS_H_
#define CORE_CLIENT_AI_ENDLESS_ENDLESS_H_

#include <memory>

#include "core/client/ai/ai.h"

struct FrameRequest;
class KumipuyoSeq;

struct EndlessResult {
    int hand;
    int score;
    int maxRensa;
    bool zenkeshi;
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
