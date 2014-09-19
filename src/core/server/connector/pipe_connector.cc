#include "core/server/connector/pipe_connector.h"

#include "core/server/connector/connector_frame_request.h"

using namespace std;

PipeConnector::PipeConnector(int playerId, int writerFd, int readerFd) :
    Connector(playerId),
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

void PipeConnector::write(const ConnectorFrameRequest& req)
{
    writeString(req.toRequestString(playerId_));
}

void PipeConnector::writeString(const string& message)
{
    fprintf(writer_, "%s\n", message.c_str());
    fflush(writer_);
    LOG(INFO) << message;
}

ConnectorFrameResponse PipeConnector::read()
{
    char buf[1000];
    char* ptr = fgets(buf, 999, reader_);
    if (!ptr)
        return ConnectorFrameResponse();

    size_t len = strlen(ptr);
    if (len == 0)
        return ConnectorFrameResponse();
    if (ptr[len-1] == '\n') {
        ptr[--len] = '\0';
    }
    if (len == 0)
        return ConnectorFrameResponse();
    if (ptr[len-1] == '\r') {
        ptr[--len] = '\0';
    }

    return ConnectorFrameResponse::parse(buf);
}
