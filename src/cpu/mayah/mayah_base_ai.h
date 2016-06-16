#ifndef CPU_MAYAH_BASE_AI_H_
#define CPU_MAYAH_BASE_AI_H_

#include "core/client/ai/ai.h"

class MayahBaseAI : public AI {
public:
    MayahBaseAI(int argc, char* argv[], const char* name) :
        AI(argc, argv, name)
    {
    }

protected:
private:
};

#endif // CPU_MAYAH_BASE_AI_H_
