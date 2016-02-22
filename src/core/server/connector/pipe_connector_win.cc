#include "core/server/connector/pipe_connector_win.h"

#define NOMINMAX
#include <windows.h>
#undef ERROR

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
unique_ptr<Connector> PipeConnectorWin::create(int playerId, const string& programName)
{
    CHECK(0 <= playerId && playerId < 10) << playerId;

    // Skip "fifo:" check

    SECURITY_ATTRIBUTES security;
    security.nLength = sizeof(security);
    security.bInheritHandle = true;
    security.lpSecurityDescriptor = nullptr;

    // Prepare a pipe from client to server.
    HANDLE hChildStdOutReader = nullptr;
    HANDLE hChildStdOutWriter = nullptr;
    if (!CreatePipe(&hChildStdOutReader, &hChildStdOutWriter, &security, 0)) {
        LOG(ERROR) << "Fail to creat pipe for stdout.";
    }
    if (!SetHandleInformation(hChildStdOutReader, HANDLE_FLAG_INHERIT, 0)) {
        LOG(ERROR) << "Stdout reader is inherited.";
    }

    // Prepare a pipe from server to client.
    HANDLE hChildStdInReader = nullptr;
    HANDLE hChildStdInWriter = nullptr;
    if (!CreatePipe(&hChildStdInWriter, &hChildStdInReader, &security, 0)) {
        LOG(ERROR) << "Fail to creat pipe for stdin.";
    }
    if (!SetHandleInformation(hChildStdInWriter, HANDLE_FLAG_INHERIT, 0)) {
        LOG(ERROR) << "Stdin writer is inherited.";
    }

    // Create child process
    STARTUPINFO startupInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.hStdError = hChildStdOutWriter;
    startupInfo.hStdOutput = hChildStdOutWriter;
    startupInfo.hStdInput = hChildStdInReader;
    startupInfo.dwFlags |= STARTF_USESTDHANDLES;

    string argument("Player_");
    argument[6] = '1' + playerId;
    string commandLine = programName + " " + argument;

    PROCESS_INFORMATION processInfo;
    ZeroMemory(&processInfo, sizeof(processInfo));
    if (!CreateProcess(programName.c_str(), const_cast<char*>(commandLine.c_str()),
                       nullptr, nullptr, true, CREATE_NO_WINDOW, nullptr, nullptr, &startupInfo, &processInfo)) {
        PLOG(FATAL) << "Failed to launch a subprocess: " << programName;
        return unique_ptr<Connector>();
    }

    return unique_ptr<Connector>(new PipeConnectorWin(playerId, hChildStdInWriter, hChildStdOutReader));
}

// static
bool PipeConnectorWin::pollAndReceive(bool waitTimeout, int frameId, const vector<PipeConnector*>& pipeConnectors, vector<FrameResponse>* cfr)
{
    TimePoint startTimePoint = Clock::now();

    HANDLE handles[NUM_PLAYERS];
    const size_t numHandles = pipeConnectors.size();
    for (size_t i = 0; i < numHandles; ++i) {
        handles[i] = static_cast<PipeConnectorWin*>(pipeConnectors[i])->reader_;
    }

    bool connection = true;
    bool receivedDataForThisFrame[NUM_PLAYERS] {};
    while (true) {
        int timeoutMs = 0;
        if (waitTimeout) {
            // Check timeout.
            timeoutMs = getRemainingMilliSeconds(startTimePoint);
            if (timeoutMs <= 0) {
                break;
            }
        }

        // Wait for user input.
        DWORD action = WaitForMultipleObjects(numHandles, handles, false, timeoutMs);

        if (action == 0) {
            if (!waitTimeout) {
                break;
            }
            continue;
        }

        if (WAIT_ABANDONED_0 <= action && action < WAIT_ABANDONED_0 + numHandles) {
            size_t i = action - WAIT_ABANDONED_0;
            PipeConnector* connector = pipeConnectors[i];
            LOG(ERROR) << "[P" << connector->playerId() << "] Closed the connection.";
            connection = false;
            connector->setClosed(true);
            continue;
        }

        CHECK(WAIT_OBJECT_0 <= action);
        CHECK(action < WAIT_OBJECT_0 + numHandles);
        size_t i = action - WAIT_OBJECT_0;
        PipeConnector* connector = pipeConnectors[i];
        FrameResponse response;
        if (connector->receive(&response)) {
            cfr[connector->playerId()].push_back(response);
            if (response.frameId == frameId) {
                receivedDataForThisFrame[i] = true;
            }
        }

        // If a realtime game flag is not set, do not wait for timeout, and
        // continue the game as soon as possible.
        if (!FLAGS_realtime) {
            bool allDataIsRead = true;
            for (size_t i = 0; i < pipeConnectors.size(); ++i) {
                if (!receivedDataForThisFrame[i]) {
                    allDataIsRead = false;
                }
            }
            if (allDataIsRead) {
                break;
            }
        }
    }


    int usec = getUsecFromStart(startTimePoint);
    LOG(INFO) << "Frame " << frameId  << " took " << usec << " [us]";

    return connection;
}

PipeConnectorWin::PipeConnectorWin(int player, HANDLE writer, HANDLE reader) :
    PipeConnector(player),
    writer_(writer),
    reader_(reader)
{
}

PipeConnectorWin::~PipeConnectorWin()
{
    CloseHandle(writer_);
    CloseHandle(reader_);
}

bool PipeConnectorWin::writeData(const void* data, size_t size)
{
    while (size > 0) {
        DWORD wroteSize;
        if (!WriteFile(writer_, data, size, &wroteSize, nullptr))
            return false;
        // TODO(mayah): data += wroteSize will do, but it's gnu extension.
        data = reinterpret_cast<const void*>(reinterpret_cast<const char*>(data) + wroteSize);
        size -= wroteSize;
    }

    FlushFileBuffers(writer_);
}

bool PipeConnectorWin::readData(void* data, size_t size)
{
    while (size > 0) {
        DWORD readSize;
        if (!ReadFile(reader_, data, size, &readSize, nullptr))
            return false;
        // TODO(mayah): data += readSize will do, but it's gnu extension.
        data = reinterpret_cast<void*>(reinterpret_cast<char*>(data) + readSize);
        size -= readSize;
    }

    return true;
}
