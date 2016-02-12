#include "core/server/connector/connector_manager_posix.h"

#include <poll.h>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/frame.h"
#include "core/frame_response.h"
#include "core/server/connector/connector.h"
#include "core/player.h"

using namespace std;

DEFINE_bool(realtime, true, "use realtime");
DEFINE_bool(no_timeout, false, "if true, wait ai's thought without timeout");

const int TIMEOUT_USEC = 1000000 / FPS;

using Clock = chrono::high_resolution_clock;
using TimePoint = Clock::time_point;

static int getUsecFromStart(const TimePoint& start)
{
    return chrono::duration_cast<chrono::microseconds>(Clock::now() - start).count();
}

static int getRemainingMilliSeconds(const TimePoint& start)
{
    if (FLAGS_no_timeout)
        return numeric_limits<int>::max();

    int usec = getUsecFromStart(start);
    return (TIMEOUT_USEC - usec + 999) / 1000;
}

ConnectorManagerPosix::ConnectorManagerPosix(unique_ptr<Connector> p1, unique_ptr<Connector> p2) :
    connectors_ { move(p1), move(p2) },
    waitTimeout_(true)
{
}

// TODO(mayah): Without polling, each connector should make thread?
// If we do so, Human connector can use MainWindow::addEventListener(), maybe.
bool ConnectorManagerPosix::receive(int frameId, vector<FrameResponse> cfr[NUM_PLAYERS])
{
    for (int i = 0; i < NUM_PLAYERS; i++)
        cfr[i].clear();

    pollfd pollfds[NUM_PLAYERS];
    int playerIds[NUM_PLAYERS];
    int numPollfds = 0;
    for (int i = 0; i < NUM_PLAYERS; i++) {
        if (connector(i)->isHuman()) {
            FrameResponse response;
            CHECK(connector(i)->receive(&response)) << "Human connector must be always receivable.";
            cfr[i].push_back(response);
            continue;
        }

        if (connector(i)->pollable()) {
            pollfds[numPollfds].fd = connector(i)->readerFd();
            pollfds[numPollfds].events = POLLIN;
            playerIds[numPollfds] = i;
            numPollfds++;
            continue;
        }

        CHECK(false) << "connector is not pollable or human. Then what's connector?";
    }
    DCHECK(numPollfds <= NUM_PLAYERS) << numPollfds;

    bool received_data_for_this_frame[NUM_PLAYERS] {};
    bool died = false;

    TimePoint startTimePoint = Clock::now();

    while (true) {
        int timeout_ms = 0;
        if (waitTimeout_) {
            // Check timeout.
            // The timeout delays for 50us in avarage (on pascal's machine)
            // Worst case is still in order of 100us, so it's OK to use gettimeofday.
            timeout_ms = getRemainingMilliSeconds(startTimePoint);
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
                FrameResponse response;
                if (connector(playerIds[i])->receive(&response)) {
                    cfr[playerIds[i]].push_back(response);
                    if (response.frameId == frameId) {
                        received_data_for_this_frame[playerIds[i]] = true;
                    }
                }
            } else if ((pollfds[i].revents & POLLERR) ||
                       (pollfds[i].revents & POLLHUP) ||
                       (pollfds[i].revents & POLLNVAL)) {
                LOG(ERROR) << "[P" << playerIds[i] << "] Closed the connection.";
                died = true;
                connector(playerIds[i])->setClosed(true);
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

    int usec = getUsecFromStart(startTimePoint);
    LOG(INFO) << "Frame " << frameId  << " took " << usec << " [us]";

    return !died;
}
