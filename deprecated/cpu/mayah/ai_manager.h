#ifndef __AI_MANAGER_H_
#define __AI_MANAGER_H_

#include <string>
#include "ai.h"
#include "protocol.h"

class AIManager {
public:
    AIManager(const std::string& name);

    int runLoop();
private:
    std::string m_name;
    AI m_ai;
    Protocol m_protocol;
};

#endif
