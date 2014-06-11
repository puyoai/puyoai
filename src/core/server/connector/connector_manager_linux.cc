#include "core/server/connector/connector_manager_linux.h"

#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <iomanip>
#include <string>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/constant.h"

using namespace std;

DEFINE_bool(realtime, true, "use realtime");

const int TIMEOUT_USEC = 1000000 / FPS;

static unique_ptr<Connector> createConnector(int playerId, const string& programName)
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

    if (execlp(programName.c_str(), filename) < 0)
        PLOG(FATAL) << "Failed to start a child process. ";

    LOG(FATAL) << "should not be reached.";
    return unique_ptr<Connector>();
}

static int GetUsecFromStart(const struct timeval& start)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec - start.tv_sec) * 1000000 + (now.tv_usec - start.tv_usec);
}

static int GetRemainingMilliSeconds(const struct timeval& start)
{
    int usec = GetUsecFromStart(start);
    return (TIMEOUT_USEC - usec + 999) / 1000;
}

ConnectorManagerLinux::ConnectorManagerLinux(vector<string> program_names) :
    waitTimeout_(true)
{
    for (int i = 0; i < program_names.size(); i++) {
        connectors_.push_back(createConnector(i, program_names[i]));
    }
}

void ConnectorManagerLinux::Write(int id, const string& message) {
    if (connectors_[id].get())
        connectors_[id]->write(message);
}

void Log(int frame_id, const vector<ReceivedData>* all_data, vector<PlayerLog>* log)
{
    // Print debug info.
    LOG(INFO) << "########## FRAME " << frame_id << " ##########";
    for (int i = 0; i < 2; i++) {
        if (all_data[i].size() == 0) {
            LOG(INFO) << "[P" << i << "] [NODATA]";
        }
        for (int j = 0; j < all_data[i].size(); j++) {
            const ReceivedData& data = all_data[i][j];
            LOG(INFO) << "[P" << i << "] "
                      << "[" << setfill(' ') << setw(5) << right << data.usec << "us] "
                      << "[" << data.original << "]";

            LOG_IF(WARNING, !data.isValid()) << "Ignoring the invalid command.";
            LOG_IF(WARNING, data.frameId > frame_id) << "Received a command for future frame.";
        }
    }

    // Fill game log.
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < all_data[i].size(); j++) {
            const ReceivedData& data = all_data[i][j];
            (*log)[i].received_data.push_back(data);
        }
    }
}

bool ConnectorManagerLinux::GetActions(int frame_id, vector<PlayerLog>* log)
{
    log->clear();
    // Initialize logging.
    log->assign(connectors_.size(), PlayerLog());
    for (int i = 0; i < connectors_.size(); i++) {
        (*log)[i].frame_id = frame_id;
        (*log)[i].player_id = i;
        (*log)[i].is_human = connectors_[i]->isHuman();
    }

    pollfd pollfds[2];
    int playerIds[2];
    int numPollfds = 0;
    for (int i = 0; i < connectors_.size(); i++) {
        if (connector(i)->pollable()) {
            pollfds[numPollfds].fd = connector(i)->readerFd();
            pollfds[numPollfds].events = POLLIN;
            playerIds[numPollfds] = i;
            numPollfds++;
        }
    }
    DCHECK(numPollfds <= 2) << numPollfds;

    vector<ReceivedData> received_data[2];
    bool received_data_for_this_frame[2] = {false, false};
    bool died = false;

    struct timeval tv_start;
    gettimeofday(&tv_start, NULL);
    while (true) {
        int timeout_ms = 0;
        if (waitTimeout_) {
            // Check timeout.
            // The timeout delays for 50us in avarage (on pascal's machine)
            // Worst case is still in order of 100us, so it's OK to use gettimeofday.
            int timeout_ms = GetRemainingMilliSeconds(tv_start);
            if (timeout_ms <= 0) {
                break;
            }
        }

        // Wait for user input.
        int actions = poll(pollfds, numPollfds, timeout_ms);

        if (actions < 0) {
            LOG(ERROR) << strerror(errno);
            break;
        } else if (actions == 0) {
            if (!waitTimeout_)
                break;
            continue;
        }

        for (int i = 0; i < numPollfds; i++) {
            if (pollfds[i].revents & POLLIN) {
                ReceivedData data = connector(playerIds[i])->read();
                if (data.received) {
                    data.usec = GetUsecFromStart(tv_start);
                    received_data[playerIds[i]].push_back(data);
                    if (data.frameId == frame_id)
                        received_data_for_this_frame[playerIds[i]] = true;
                }
            } else if ((pollfds[i].revents & POLLERR) ||
                       (pollfds[i].revents & POLLHUP) ||
                       (pollfds[i].revents & POLLNVAL)) {
                LOG(ERROR) << "[P" << playerIds[i] << "] Closed the connection.";
                died = true;
                connector(playerIds[i])->setAlive(false);
            }
        }

        // If a realtime game flag is not set, do not wait for timeout, and
        // continue the game as soon as possible.
        if (!FLAGS_realtime) {
            bool all_data_is_read = true;
            for (int i = 0; i < 2; i++) {
                if (!received_data_for_this_frame[i]) {
                    all_data_is_read = false;
                }
            }
            if (all_data_is_read) {
                break;
            }
        }
    }

    Log(frame_id, received_data, log);

    return !died;
}

string ConnectorManagerLinux::GetErrorLog()
{
    // TODO(mayah): Implement this or remove this function at all.
    return "";
}
