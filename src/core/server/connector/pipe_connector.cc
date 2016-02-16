#include "core/server/connector/pipe_connector.h"

#include <cstring>
#include <string>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/frame_request.h"
#include "core/frame_response.h"

#if defined(_MSC_VER)
#include "core/server/connector/pipe_connector_win.h"
#else
#include "core/server/connector/pipe_connector_posix.h"
#endif

using namespace std;

// static
unique_ptr<Connector> PipeConnector::create(int playerId, const string& programName)
{
#if defined(_MSC_VER)
    return PipeConnectorWin::create(playerId, programName);
#else
    return PipeConnectorPosix::create(playerId, programName);
#endif
}

PipeConnector::PipeConnector(int player) :
    Connector(player),
    closed_(false)
{
}

void PipeConnector::send(const FrameRequest& req)
{
    writeString(req.toString());
}

bool PipeConnector::receive(FrameResponse* response)
{
    // TODO(peria): Consider design of this routine.  Use std::string instead?
    char buffer[kBufferSize];
    if (!readString(buffer))
        return false;

    size_t len = strlen(buffer);
    if (len == 0)
        return false;

    if (buffer[len - 1] == '\n') {
        buffer[--len] = '\0';
    }
    if (len == 0)
        return false;
    if (buffer[len - 1] == '\r') {
        buffer[--len] = '\0';
    }

    LOG(INFO) << buffer;
    *response = FrameResponse::parse(buffer);
    return true;
}
