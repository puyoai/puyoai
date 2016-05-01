#include "core/server/connector/server_connector.h"

#include <string>

#include <glog/logging.h>

#include "core/server/connector/human_connector.h"

#ifdef OS_WIN
# include "core/server/connector/pipe_connector_win.h"
#else
# include "core/server/connector/pipe_connector_posix.h"
#endif

using namespace std;

// static
unique_ptr<ServerConnector> ServerConnector::create(int playerId, const string& programName)
{
    CHECK(0 <= playerId && playerId < 10) << playerId;

    if (programName == "-")
        return unique_ptr<ServerConnector>(new HumanConnector(playerId));

#ifdef OS_WIN
    return PipeConnectorWin::create(playerId, programName);
#else
    return PipeConnectorPosix::create(playerId, programName);
#endif
}
