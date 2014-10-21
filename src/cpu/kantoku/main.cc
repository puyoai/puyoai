#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/client/connector/client_connector.h"
#include "core/frame_request.h"
#include "core/frame_response.h"
#include "core/server/connector/connector.h"

using namespace std;

class ChildAI {
public:
    ChildAI(const string& name, const string& program) :
        name_(name)
    {
        connector_ = std::move(Connector::create(1, program));
    }

    Connector& connector() { return *connector_; }

private:
    string name_;
    unique_ptr<Connector> connector_;
};

class Kantoku {
public:
    Kantoku() : rnd_(random_device()())
    {
    }

    void add(const string& name, const string& program)
    {
        ais_.emplace_back(name, program);
    }

    void runLoop()
    {
        while (true) {
            FrameRequest req = clientConnector_.receive();
            if (req.connectionLost)
                break;

            if (req.frameId == 1) {
                chooseAI();
            }

            current().connector().write(req);
            FrameResponse resp = current().connector().read();

            clientConnector_.send(resp);
        }
    }

private:
    void chooseAI()
    {
        DCHECK_LT(0UL, ais_.size());
        index_ = uniform_int_distribution<int>(0, ais_.size() - 1)(rnd_);
    }

    ChildAI& current() { return ais_[index_]; }

    default_random_engine rnd_;
    ClientConnector clientConnector_;
    int index_;
    vector<ChildAI> ais_;
};

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    Kantoku kantoku;
    kantoku.add("mayah", "../mayah/run.sh");

    kantoku.runLoop();

    return 0;
}
