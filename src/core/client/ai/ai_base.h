#ifndef CORE_CLIENT_AI_BASE_H_
#define CORE_CLIENT_AI_BASE_H_

#include <memory>

#include "core/client/client_connector.h"

class AIBase {
public:
    virtual ~AIBase() {}

protected:
    static std::unique_ptr<ClientConnector> makeConnector();
};

#endif // CORE_CLIENT_AI_BASE_H_
