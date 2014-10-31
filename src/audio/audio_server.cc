#include "audio/audio_server.h"

#include <glog/logging.h>

#include "audio/audio_server.h"
#include "audio/speaker.h"

using namespace std;

AudioServer::AudioServer(Speaker* speaker) :
    speaker_(speaker),
    shouldStop_(false)
{
}

AudioServer::~AudioServer()
{
    stop();
}

void AudioServer::start()
{
    th_ = thread([this](){
        this->runLoop();
    });
}

void AudioServer::stop()
{
    shouldStop_ = true;
    cond_.notify_all();
    if (th_.joinable())
        th_.join();
}

void AudioServer::submit(const std::string& text, int priority, int timeout)
{
    LOG(INFO) << "audio submitted: " << text;

    unique_lock<mutex> lock(mu_);

    chrono::system_clock::time_point p = chrono::system_clock::now();
    p += chrono::seconds(timeout);

    requests_.emplace(text, priority, p);
    cond_.notify_one();
}

void AudioServer::runLoop()
{
    LOG(INFO) << "hogehoge";
    while (!shouldStop_) {
        LOG(INFO) << "hoge";
        SpeakRequest request = take();
        LOG(INFO) << "fuga";
        if (request.text.empty())
            continue;

        chrono::system_clock::time_point p = chrono::system_clock::now();
        if (request.timeoutPoint < p) {
            LOG(INFO) << "timeout: " << request.text;
            continue;
        }

        LOG(INFO) << "Speak: " << request.text;
        speaker_->speak(request.text);
    }
}

AudioServer::SpeakRequest AudioServer::take()
{
    unique_lock<mutex> lock(mu_);
    while (requests_.empty()) {
        if (shouldStop_)
            return SpeakRequest();
        cond_.wait(lock);
    }

    SpeakRequest request = requests_.top();
    requests_.pop();
    return request;
}

