#include "core/server/connector/connector_manager_linux.h"

#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <string.h>
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

ConnectorManagerLinux::ConnectorManagerLinux(unique_ptr<Connector> p1, unique_ptr<Connector> p2) :
    connectors_ { move(p1), move(p2) },
    waitTimeout_(true)
{
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

// TODO(mayah): Without polling, each connector should make thread?
// If we do so, Human connector can use MainWindow::addEventListener(), maybe.
bool ConnectorManagerLinux::GetActions(int frame_id, vector<PlayerLog>* log)
{
    log->clear();
    // Initialize logging.
    log->assign(NUM_PLAYERS, PlayerLog());
    for (int i = 0; i < NUM_PLAYERS; i++) {
        (*log)[i].frame_id = frame_id;
        (*log)[i].player_id = i;
    }

    pollfd pollfds[NUM_PLAYERS];
    int playerIds[NUM_PLAYERS];
    int numPollfds = 0;
    for (int i = 0; i < NUM_PLAYERS; i++) {
        if (connector(i)->pollable()) {
            pollfds[numPollfds].fd = connector(i)->readerFd();
            pollfds[numPollfds].events = POLLIN;
            playerIds[numPollfds] = i;
            numPollfds++;
        }
    }
    DCHECK(numPollfds <= NUM_PLAYERS) << numPollfds;

    vector<ReceivedData> received_data[NUM_PLAYERS];
    bool received_data_for_this_frame[NUM_PLAYERS] {};
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
