#include "connector.h"

#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <cstdio>
#include <sstream>
#include <string>

#include <glog/logging.h>

using namespace std;

// static
unique_ptr<Connector> Connector::create(int playerId, const string& programName)
{
    if (programName == "-")
        return unique_ptr<Connector>(new HumanConnector);

    if (programName.find("fifo:") == 0) {
        string::size_type colon = programName.find(":", 5);
        string uplink_fifo = programName.substr(5, colon - 5);
        string downlink_fifo = programName.substr(colon + 1);
        CHECK(mkfifo(uplink_fifo.c_str(), 0777) == 0);
        CHECK(chmod(uplink_fifo.c_str(), 0777) == 0);
        CHECK(mkfifo(downlink_fifo.c_str(), 0777) == 0);
        CHECK(chmod(downlink_fifo.c_str(), 0777) == 0);
        int uplink_fd = open(uplink_fifo.c_str(), O_RDONLY);
        CHECK(uplink_fd >= 0);
        int downlink_fd = open(downlink_fifo.c_str(), O_WRONLY);
        CHECK(downlink_fd >= 0);

        unique_ptr<Connector> connector(new PipeConnector(downlink_fd, uplink_fd));
        connector->write("PingMessage");
        (void)connector->read();

        return connector;
    }

    // File descriptors.
    int fd_field_status[2];
    int fd_command[2];
    int fd_cpu_error[2];

    if (pipe(fd_field_status)) {
        LOG(FATAL) << "Pipe error. " << strerror(errno);
    }
    if (pipe(fd_command)) {
        LOG(FATAL) << "Pipe error. " << strerror(errno);
    }
    if (pipe(fd_cpu_error)) {
        LOG(FATAL) << "Pipe error. " << strerror(errno);
    }

    pid_t pid = fork();
    if (pid < 0) {
        LOG(FATAL) << "Failed to fork. " << strerror(errno);
    }

    if (pid > 0) {
        // Server.
        LOG(INFO) << "Created a child process (pid = " << pid << ")";

        unique_ptr<Connector> connector(new PipeConnector(fd_field_status[1], fd_command[0]));
        close(fd_field_status[0]);
        close(fd_command[1]);
        close(fd_cpu_error[1]);

        connector->write("PingMessage");
        (void)connector->read();

        return connector;
    }

    // Client.
    if (dup2(fd_field_status[0], STDIN_FILENO) == -1) {
        LOG(FATAL) << "Failed to dup2. " << strerror(errno);
    }
    if (dup2(fd_command[1], STDOUT_FILENO) == -1) {
        LOG(FATAL) << "Failed to dup2. " << strerror(errno);
    }
    if (dup2(fd_cpu_error[1], STDERR_FILENO) == -1) {
        LOG(FATAL) << "Failed to dup2. " << strerror(errno);
    }

    close(fd_field_status[0]);
    close(fd_field_status[1]);
    close(fd_command[0]);
    close(fd_command[1]);
    close(fd_cpu_error[0]);
    close(fd_cpu_error[1]);

    char filename[] = "Player1";
    filename[6] += playerId; // TODO(mayah): What's this !!

    if (execl(programName.c_str(), filename, nullptr) < 0)
        PLOG(FATAL) << "Failed to start a child process. ";

    LOG(FATAL) << "should not be reached.";
    return unique_ptr<Connector>();
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
