#include "core/server/connector/pipe_connector_win.h"

#include <chrono>
#include <cstddef>
#include <cstring>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/frame_request.h"
#include "core/frame_response.h"

using namespace std;

DEFINE_bool(realtime, true, "use realtime");

// static
unique_ptr<Connector> PipeConnectorWin::create(int, const string&)
{
    LOG(FATAL) << "TODO(peria): Implement here";
    return unique_ptr<Connector>();
}

// static
bool PipeConnectorWin::pollAndReceive(bool waitTimeout, int frameId, const vector<PipeConnector*>& pipeConnectors, vector<FrameResponse>* cfr)
{
    LOG(FATAL) << "TODO(peria): Implement here";
    return false;
}

PipeConnectorWin::PipeConnectorWin(int player, int, int) :
    PipeConnector(player)
{
}

PipeConnectorWin::~PipeConnectorWin()
{
}

void PipeConnectorWin::writeString(const string&)
{
    LOG(FATAL) << "TODO(peria): Implement here";
}

bool PipeConnectorWin::readString(char*)
{
    LOG(FATAL) << "TODO(peria): Implement here";
    return false;
}
