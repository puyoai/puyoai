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

DEFINE_bool(change_if_beated, true,
            "If true, AI will be changed when beated. If false, AI will be changed by one game");
DEFINE_bool(change_random, false,
            "If true, AI will be changed randomly. Otherwise, sequencially selected.");

using namespace std;

class ChildAI {
public:
    ChildAI(const string& name, const string& program) :
        name_(name)
    {
        connector_ = std::move(Connector::create(1, program));
    }

    const std::string& name() const { return name_; }
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
        // When this AI is beated, next AI should be chosen.
        if (req.matchEnd) {
            if (req.gameResult == GameResult::P2_WIN || req.gameResult == GameResult::P2_WIN_WITH_CONNECTION_ERROR)
                shouldChooseAI_ = true;
        }

        if (req.shouldInitialize()) {
            // Always change the ai.
            if (!FLAGS_change_if_beated)
                shouldChooseAI_ = true;

            if (shouldChooseAI_) {
                shouldChooseAI_ = false;
                chooseAI();
            }
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
        if (FLAGS_change_random) {
            index_ = uniform_int_distribution<int>(0, ais_.size() - 1)(rnd_);
        } else {
            index_ = (index_ + 1) % ais_.size();
        }

        LOG(INFO) << "Current CPU: " << current().name();
    }

    ChildAI& current()
    {
        return ais_[index_];
    }

    default_random_engine rnd_;
    bool shouldChooseAI_ = false;
    int index_;
    vector<ChildAI> ais_;
};

int main(int argc, char* argv[])
{
    google::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    Kantoku kantoku;
    kantoku.add("mayah-1", "../mayah/run.sh");
    kantoku.add("nidub", "../test_lockit/nidub.sh");
    kantoku.add("mayah-2", "../mayah/run.sh");
    kantoku.add("rendaS9", "../test_lockit/rendaS9.sh");
    kantoku.add("mayah-3", "../mayah/run.sh");
    kantoku.add("rendaGS9", "../test_lockit/rendaGS9.sh");

    kantoku.runLoop();

    return 0;
}
