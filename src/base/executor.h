#ifndef BASE_EXECUTOR_H_
#define BASE_EXECUTOR_H_

#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

class Executor {
public:
    typedef std::function<void (void)> Func;

    Executor(int numThread);
    ~Executor();

    void start();
    void stop();

    void submit(Func);

private:
    void runWorkerLoop();
    Func take();

    std::vector<std::thread> threads_;
    std::mutex mu_;
    std::condition_variable condVar_;
    std::atomic<bool> shouldStop_;
    std::deque<Func> tasks_;
};

#endif
