#include "audio/audio_server.h"

#include <glog/logging.h>

#include "audio/audio_server.h"
#include "audio/speaker.h"
#include "audio/speak_requester.h"

using namespace std;

AudioServer::AudioServer(Speaker* speaker) :
    speaker_(speaker)
{
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

void AudioServer::addSpeakRequester(SpeakRequester* requester)
{
    lock_guard<mutex> lock(mu_);
    speakRequesters_.push_back(requester);
}

void AudioServer::addText(const std::string& text)
{
    unique_lock<mutex> lock(mu_);
    texts_.push_back(text);
    cond_.notify_one();
}

void AudioServer::runLoop()
{
    while (!shouldStop_) {
        for (SpeakRequester* requester : speakRequesters_) {
            SpeakRequest req = requester->requestSpeak();
            if (!req.text.empty()) {
                addText(req.text);
            }
        }

        string textToSpeak;
        {
            unique_lock<mutex> lock(mu_);
            if (texts_.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }

            if (!texts_.empty()) {
                textToSpeak = texts_.back();
                texts_.pop_back();
                texts_.clear();
            }
        }

        LOG(INFO) << "Speak: " << textToSpeak;
        if (speaker_)
            speaker_->speak(textToSpeak);
    }
}

