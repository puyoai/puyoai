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

namespace {

const int TIMEOUT_USEC = 1000000 / FPS;

}

DEFINE_bool(no_timeout, false, "if true, wait ai's thought without timeout");

// static
unique_ptr<ServerConnector> PipeConnector::create(int playerId, const string& programName)
{
#if defined(_MSC_VER)
    return PipeConnectorWin::create(playerId, programName);
#else
    return PipeConnectorPosix::create(playerId, programName);
#endif
}

// static
int PipeConnector::getUsecFromStart(const TimePoint& start)
{
    return chrono::duration_cast<chrono::microseconds>(Clock::now() - start).count();
}

// static
int PipeConnector::getRemainingMilliSeconds(const TimePoint& start)
{
    if (FLAGS_no_timeout)
        return numeric_limits<int>::max();

    int usec = getUsecFromStart(start);
    return (TIMEOUT_USEC - usec + 999) / 1000;
}

PipeConnector::PipeConnector(int player) :
    ServerConnector(player),
    closed_(false)
{
}

void PipeConnector::send(const FrameRequest& req)
{
    std::string s = req.toString();

    // Send header first.
    FrameRequestHeader header(s.size());
    if (!writeData(reinterpret_cast<const void*>(&header), sizeof(header))) {
        LOG(ERROR) << "failed to write message header";
        return;
    }

    if (!writeData(reinterpret_cast<const void*>(s.data()), s.size())) {
        LOG(ERROR) << "failed to write message payload";
        return;
    }
}

bool PipeConnector::receive(FrameResponse* response)
{
    // Receives header.
    FrameResponseHeader header;
    if (!readData(reinterpret_cast<void*>(&header), sizeof(header))) {
        LOG(ERROR) << "failed to read message header";
        return false;
    }

    if (header.size > kBufferSize) {
        LOG(ERROR) << "body is too large to read: size=" << header.size;
        return false;
    }

    char payload[kBufferSize];
    if (!readData(reinterpret_cast<void*>(payload), header.size)) {
        LOG(ERROR) << "failed to read payload";
        return false;
    }

    *response = FrameResponse::parsePayload(payload, header.size);
    LOG(INFO) << "RECEIVED: " << response->toString();
    return true;
}
