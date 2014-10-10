#include "base/executor.h"

#include <glog/logging.h>

using namespace std;

Executor::Executor(int numThread) :
    threads_(numThread),
    shouldStop_(false)
{
}

Executor::~Executor()
{
    stop();
}

void Executor::start()
{
    for (size_t i = 0; i < threads_.size(); ++i) {
        threads_[i] = thread([this]() {
                runWorkerLoop();
        });
    }
}

void Executor::stop()
{
    shouldStop_ = true;
    condVar_.notify_all();
    for (size_t i = 0; i < threads_.size(); ++i) {
        if (threads_[i].joinable()) {
            threads_[i].join();
        }
    }
}

void Executor::submit(Executor::Func f)
{
    CHECK(f) << "function should be callable";

    unique_lock<mutex> lock(mu_);
    tasks_.push_back(f);
    condVar_.notify_one();
}

void Executor::runWorkerLoop()
{
    while (true) {
        Func f = take();
        if (!f)
            break;

        f();
    }
}

Executor::Func Executor::take()
{
    unique_lock<mutex> lock(mu_);
    while (true) {
        if (!tasks_.empty())
            break;
        if (shouldStop_)
            return Func();
        condVar_.wait(lock);
    }

    Func f = tasks_.front();
    tasks_.pop_front();
    return f;
}
