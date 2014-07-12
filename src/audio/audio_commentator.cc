#include "audio/audio_commentator.h"

#include <iostream>
#include "glog/logging.h"

#include "audio/speaker.h"

using namespace std;

AudioCommentator::AudioCommentator(Speaker* speaker) :
    speaker_(speaker)
{
}

AudioCommentator::~AudioCommentator()
{
    stop();
}

void AudioCommentator::newGameWillStart()
{
    LOG(INFO) << "new game will start";
    addText("さあ、ゲームの始まりです。");
}

void AudioCommentator::onUpdate(const GameState&)
{
}

void AudioCommentator::gameHasDone()
{
}

void AudioCommentator::start()
{
    th_ = thread([this](){
        this->runLoop();
    });
}

void AudioCommentator::stop()
{
    shouldStop_ = true;
    cond_.notify_all();
    if (th_.joinable())
        th_.join();
}

void AudioCommentator::addText(const std::string& text)
{
    unique_lock<mutex> lock(mu_);
    texts_.push_back(text);
    cond_.notify_one();
}

void AudioCommentator::runLoop()
{
    while (!shouldStop_) {
        unique_lock<mutex> lock(mu_);
        if (texts_.empty())
            cond_.wait(lock);

        if (texts_.empty())
            continue;

        string text = texts_.back();
        texts_.pop_back();
        texts_.clear();

        if (speaker_)
            speaker_->speak(text);
    }
}
