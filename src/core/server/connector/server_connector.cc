#include "core/server/connector/server_connector.h"

#include <string>

#include <glog/logging.h>

#include "core/server/connector/human_connector.h"
#include "core/server/connector/pipe_connector.h"

using namespace std;

// static
unique_ptr<ServerConnector> ServerConnector::create(int playerId, const string& programName)
{
    CHECK(0 <= playerId && playerId < 10) << playerId;

    if (programName == "-")
        return unique_ptr<ServerConnector>(new HumanConnector(playerId));

    return PipeConnector::create(playerId, programName);
}
