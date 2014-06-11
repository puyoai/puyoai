#include "connector.h"

#include <string.h>

#include <cstdio>
#include <sstream>
#include <string>

#include <glog/logging.h>

Connector::Connector() :
    writer_fd_(0),
    reader_fd_(0),
    writer_(NULL),
    reader_(NULL)
{
}

Connector::Connector(int writer_fd, int reader_fd)
{
    writer_fd_ = writer_fd;
    reader_fd_ = reader_fd;
    writer_ = fdopen(writer_fd, "w");
    reader_ = fdopen(reader_fd, "r");
}

void Connector::Write(const std::string& message)
{
    if (!writer_)
        return;

    std::fprintf(writer_, "%s\n", message.c_str());
    std::fflush(writer_);

    LOG(INFO) << message;
}

ReceivedData Connector::Read()
{
    if (!reader_)
        return ReceivedData();

    char buf[1000];
    char* ptr = fgets(buf, 999, reader_);
    if (!ptr)
        return ReceivedData();

    size_t len = strlen(ptr);
    if (len == 0)
        return ReceivedData();
    if (ptr[len-1] == '\n') {
        ptr[--len] = '\0';
    }
    if (len == 0)
        return ReceivedData();
    if (ptr[len-1] == '\r') {
        ptr[--len] = '\0';
    }

    return parse(buf);
}

ReceivedData Connector::parse(const char* str)
{
    std::istringstream iss(str);
    std::string tmp;

    ReceivedData data;

    data.received = true;
    data.original = std::string(str);
    data.original = data.original.substr(0, data.original.size() - 1);  // What's this? chomp?

    while (getline(iss, tmp, ' ')) {
        if (tmp.substr(0, 3) == "ID=") {
            std::istringstream istr(tmp.c_str() + 3);
            istr >> data.frameId;
        } else if (tmp.substr(0, 2) == "X=") {
            std::istringstream istr(tmp.c_str() + 2);
            istr >> data.decision.x;
        } else if (tmp.substr(0, 2) == "R=") {
            std::istringstream istr(tmp.c_str() + 2);
            istr >> data.decision.r;
        } else if (tmp.substr(0, 4) == "MSG=") {
            data.msg = tmp.c_str() + 4;
        } else if (tmp.substr(0, 3) == "MA=") {
            data.mawashi_area = tmp.c_str() + 3;
        }
    }
    //data->status = OK;

    return data;
}
