#include "core/client/ai/ai_base.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/client/connector/stdio_client_connector.h"

// static
std::unique_ptr<ClientConnector> AIBase::makeConnector()
{
    return std::unique_ptr<ClientConnector>(new StdioClientConnector);
}
