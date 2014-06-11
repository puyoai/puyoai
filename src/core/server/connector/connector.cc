#include "connector.h"

#include <string.h>

#include <cstdio>
#include <sstream>
#include <string>

#include <glog/logging.h>

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

PipeConnector::PipeConnector(int writerFd, int readerFd)
{
    writerFd_ = writerFd;
    readerFd_ = readerFd;
    writer_ = fdopen(writerFd, "w");
    reader_ = fdopen(readerFd, "r");

    CHECK(writer_);
    CHECK(reader_);
}

PipeConnector::~PipeConnector()
{
    fclose(writer_);
    fclose(reader_);
}

void PipeConnector::write(const std::string& message)
{
    fprintf(writer_, "%s\n", message.c_str());
    fflush(writer_);

    LOG(INFO) << message;
}

ReceivedData PipeConnector::read()
{
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

void HumanConnector::write(const std::string& message)
{
    LOG(INFO) << message;
}

ReceivedData HumanConnector::read()
{
    return ReceivedData();
}

void HumanConnector::setAlive(bool)
{
    CHECK(false) << "HumanConnector does not have alive flag.";
}

int HumanConnector::readerFd() const
{
    CHECK(false) << "HumanConnector does not have reader file descriptor.";
    return -1;
}
