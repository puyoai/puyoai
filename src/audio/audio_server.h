#ifndef AUDIO_AUDIO_SERVER_H_
#define AUDIO_AUDIO_SERVER_H_

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

class Speaker;

class AudioServer {
public:
    // Doesn't take ownership.
    explicit AudioServer(Speaker*);
    ~AudioServer();

    void start();
    void stop();

    // Submits a new text to speak. If |timeout| [s] passed before |text| is
    // spoken, that text will be lost.
    void submit(const std::string& text, int priority = 0, int timeout = 5);

private:
    struct SpeakRequest {
        SpeakRequest() : priority(), timeoutPoint() {}
        SpeakRequest(const std::string& text, int priority, std::chrono::system_clock::time_point timeoutPoint) :
            text(text), priority(priority), timeoutPoint(timeoutPoint) {}

        friend bool operator<(const SpeakRequest& lhs, const SpeakRequest& rhs)
        {
            return std::tie(lhs.priority, lhs.timeoutPoint, lhs.text) <
                std::tie(rhs.priority, rhs.timeoutPoint, rhs.text);
        };

        std::string text;
        int priority;
        std::chrono::system_clock::time_point timeoutPoint;
    };

    SpeakRequest take();
    void runLoop();

    Speaker* speaker_ = nullptr;
    std::atomic<bool> shouldStop_;
    std::thread th_;
    std::mutex mu_;
    std::condition_variable cond_;
    std::priority_queue<SpeakRequest> requests_;
};

#endif
