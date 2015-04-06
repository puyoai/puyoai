#include "base/executor.h"

#include <gflags/gflags.h>
#include <glog/logging.h>

DEFINE_int32(num_threads, 1, "The default number of threads");

using namespace std;

// static
unique_ptr<Executor> Executor::makeDefaultExecutor(bool automaticStart)
{
    Executor* executor = new Executor(FLAGS_num_threads);
    if (automaticStart)
        executor->start();

    return unique_ptr<Executor>(executor);
}

Executor::Executor(int numThread) :
    threads_(numThread),
    shouldStop_(false),
    hasStarted_(false)
{
}

Executor::~Executor()
{
    if (hasStarted_)
        stop();
}

void Executor::start()
{
    CHECK(!hasStarted_);
    hasStarted_ = true;

    for (size_t i = 0; i < threads_.size(); ++i) {
        threads_[i] = thread([this]() {
                runWorkerLoop();
        });
    }
}

void Executor::stop()
{
    CHECK(hasStarted_);

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

    Func f = std::move(tasks_.front());
    tasks_.pop_front();
    return f;
}
