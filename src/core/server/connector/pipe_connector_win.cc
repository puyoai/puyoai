#include "core/server/connector/pipe_connector_win.h"

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

// static
unique_ptr<ServerConnector> PipeConnectorWin::create(int playerId, const string& programName)
{
    CHECK(0 <= playerId && playerId < 10) << playerId;

    // Skip "fifo:" check

    // Create two pipes between server (duel) and clients.
    // Server  --(writer)====[[StdIn]]====(reader)--> Client
    // Server <--(reader)===[[StdOut]]====(writer)--  Client
    // Server uses StdInWriter and StdOutReader handles,
    // and clients use StdInReader and StdOutWriter.
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
    // Ensure the read handle to the pipe for STDOUT is not inherited.
    if (!SetHandleInformation(hChildStdOutReader, HANDLE_FLAG_INHERIT, 0)) {
        LOG(ERROR) << "Fail to prohibit inheritance of stdout reader.";
    }

    // Prepare a pipe from server to client.
    HANDLE hChildStdInReader = nullptr;
    HANDLE hChildStdInWriter = nullptr;
    if (!CreatePipe(&hChildStdInReader, &hChildStdInWriter, &security, 0)) {
        LOG(ERROR) << "Fail to creat pipe for stdin.";
    }
    // Ensure the write handle to the pipe for STDIN is not inherited.
    if (!SetHandleInformation(hChildStdInWriter, HANDLE_FLAG_INHERIT, 0)) {
        LOG(ERROR) << "Fail to prohibit inheritance of stdin writer.";
    }

    // Create child process
    STARTUPINFO startupInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
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
        return unique_ptr<ServerConnector>();
    }

    return unique_ptr<ServerConnector>(new PipeConnectorWin(playerId, hChildStdInWriter, hChildStdOutReader));
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

        data = reinterpret_cast<const void*>(reinterpret_cast<const char*>(data) + wroteSize);
        size -= wroteSize;
    }

    return true;
}

bool PipeConnectorWin::readData(void* data, size_t size)
{
    while (size > 0) {
        DWORD readSize;
        if (!ReadFile(reader_, data, size, &readSize, nullptr))
            return false;

        data = reinterpret_cast<void*>(reinterpret_cast<char*>(data) + readSize);
        size -= readSize;
    }

    return true;
}
