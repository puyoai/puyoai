#include "base/wait_group.h"

#include <glog/logging.h>

using namespace std;

WaitGroup::WaitGroup() :
    num_(0)
{
}

void WaitGroup::add(int n)
{
    lock_guard<mutex> lock(mu_);
    num_ += n;
}

void WaitGroup::done()
{
    lock_guard<mutex> lock(mu_);
    CHECK_GT(num_, 0) << "Probably you forgot calling add().";

    --num_;

    if (num_ == 0)
        condVar_.notify_one();
}

void WaitGroup::waitUntilDone()
{
    unique_lock<mutex> lock(mu_);
    while (num_ > 0) {
        condVar_.wait(lock);
    }
}
