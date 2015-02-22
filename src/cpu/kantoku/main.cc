#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "core/client/ai/raw_ai.h"
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

    std::string name() const { return name_; }

    Connector& connector() { return *connector_; }

private:
    string name_;
    unique_ptr<Connector> connector_;
};

class Kantoku : public RawAI {
public:
    Kantoku() : rnd_(random_device()()), index_(0)
    {
    }

    void add(const string& name, const string& program)
    {
        ais_.emplace_back(name, program);
    }

    FrameResponse playOneFrame(const FrameRequest& req) override
    {
        if (req.shouldInitialize()) {
            chooseAI();
        }

        current().connector().send(req);
        FrameResponse response;
        CHECK(current().connector().receive(&response));
        return response;
    }

private:
    void chooseAI()
    {
        DCHECK_LT(0UL, ais_.size());
        index_ = uniform_int_distribution<int>(0, ais_.size() - 1)(rnd_);

        LOG(INFO) << "Current CPU: " << current().name();
    }

    ChildAI& current()
    {
        return ais_[index_];
    }

    default_random_engine rnd_;
    int index_;
    vector<ChildAI> ais_;
};

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    Kantoku kantoku;
    kantoku.add("mayah-1", "../mayah/run_advanced.sh");
    kantoku.add("mayah-2", "../mayah/run_advanced.sh");
    kantoku.add("mayah-3", "../mayah/run_advanced.sh");
    kantoku.add("nidub", "../../internal/cpu/test_lockit/nidub.sh");
    kantoku.add("rendaS9", "../../internal/cpu/test_lockit/rendaS9.sh");
    kantoku.add("rendaGS9", "../../internal/cpu/test_lockit/rendaGS9.sh");

    kantoku.runLoop();

    return 0;
}
