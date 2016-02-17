#include "core/server/connector/pipe_connector_win.h"

#include <chrono>
#include <cstddef>
#include <cstring>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/frame_request.h"
#include "core/frame_response.h"

using namespace std;

DECLARE_bool(realtime);
DECLARE_bool(no_timeout);

namespace {

using Clock = chrono::high_resolution_clock;
using TimePoint = Clock::time_point;

const int TIMEOUT_USEC = 1000000 / FPS;

int getUsecFromStart(const TimePoint& start)
{
    return chrono::duration_cast<chrono::microseconds>(Clock::now() - start).count();
}

int getRemainingMilliSeconds(const TimePoint& start)
{
    if (FLAGS_no_timeout)
        return numeric_limits<int>::max();

    int usec = getUsecFromStart(start);
    return (TIMEOUT_USEC - usec + 999) / 1000;
}

} // namespace

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
