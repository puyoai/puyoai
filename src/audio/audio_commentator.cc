#include "audio/audio_commentator.h"

#include <iostream>
#include "glog/logging.h"

#include "audio/speaker.h"
#include "duel/game_state.h"

using namespace std;

AudioCommentator::AudioCommentator(Speaker* speaker) :
    needsUpdate_ {},
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

void AudioCommentator::onUpdate(const GameState& gameState)
{
    lock_guard<mutex> lock(mu_);

    for (int pi = 0; pi < 2; ++pi) {
        const FieldRealtime& fr = gameState.field(pi);
        if (!fr.userState().grounded)
            continue;

        field_[pi] = fr.field();
        field_[pi].forceDrop();
        kumipuyoSeq_[pi] = fr.kumipuyoSeq();
        needsUpdate_[pi] = true;
    }
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
        {
            unique_lock<mutex> lock(mu_);
            if (texts_.empty() || needsUpdate_[0] || needsUpdate_[1])
                cond_.wait(lock);
        }

        updateFieldIfNecessary();
        speakIfNecessary();
    }
}

void AudioCommentator::updateFieldIfNecessary()
{
    // TODO(mayah): Implement this.
}

void AudioCommentator::speakIfNecessary()
{
    string textToSpeak;
    {
        unique_lock<mutex> lock(mu_);
        if (texts_.empty())
            return;

        textToSpeak = texts_.back();
        texts_.pop_back();
        texts_.clear();
    }

    if (speaker_)
        speaker_->speak(textToSpeak);
}
