#include "core/server/connector/pipe_connector.h"

#include <glog/logging.h>

#include <cstddef>
#include <cstring>

#include "core/frame_request.h"
#include "core/frame_response.h"

using namespace std;

PipeConnector::PipeConnector(int writerFd, int readerFd) :
    writerFd_(writerFd),
    readerFd_(readerFd)
{
    writer_ = fdopen(writerFd_, "w");
    reader_ = fdopen(readerFd_, "r");

    CHECK(writer_);
    CHECK(reader_);
}

PipeConnector::~PipeConnector()
{
    fclose(writer_);
    fclose(reader_);
}

void PipeConnector::send(const FrameRequest& req)
{
    writeString(req.toString());
}

void PipeConnector::writeString(const string& message)
{
    fprintf(writer_, "%s\n", message.c_str());
    fflush(writer_);
    LOG(INFO) << message;
}

bool PipeConnector::receive(FrameResponse* response)
{
    char buf[1000];
    char* ptr = fgets(buf, 999, reader_);
    if (!ptr)
        return false;

    size_t len = strlen(ptr);
    if (len == 0)
        return false;

    if (ptr[len-1] == '\n') {
        ptr[--len] = '\0';
    }
    if (len == 0)
        return false;
    if (ptr[len-1] == '\r') {
        ptr[--len] = '\0';
    }

    LOG(INFO) << buf;
    *response = FrameResponse::parse(buf);
    return true;
}
