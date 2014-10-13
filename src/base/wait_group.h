#ifndef BASE_WAITGROUP_H_
#define BASE_WAITGROUP_H_

#include <condition_variable>
#include <mutex>

class WaitGroup {
public:
    WaitGroup();

    void add(int n);
    void done();

    void waitUntilDone();

private:
    std::mutex mu_;
    std::condition_variable condVar_;
    int num_;
};

#endif
