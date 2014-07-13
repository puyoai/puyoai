#ifndef AUDIO_AUDIO_SERVER_H_
#define AUDIO_AUDIO_SERVER_H_

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class Speaker;
class SpeakRequester;

class AudioServer {
public:
    explicit AudioServer(Speaker*);

    void start();
    void stop();

    void addSpeakRequester(SpeakRequester*);
    void addText(const std::string&);

private:
    void runLoop();

    Speaker* speaker_ = nullptr;
    std::atomic<bool> shouldStop_;
    std::thread th_;
    std::mutex mu_;
    std::condition_variable cond_;
    std::vector<std::string> texts_;
    std::vector<SpeakRequester*> speakRequesters_;
};

#endif
