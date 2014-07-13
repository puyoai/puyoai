#include "audio/audio_commentator.h"

#include <iostream>
#include "glog/logging.h"

#include "audio/speaker.h"
#include "duel/game_state.h"
#include "gui/commentator.h"

using namespace std;

AudioCommentator::AudioCommentator(const Commentator* commentator) :
    commentator_(commentator)
{
}

AudioCommentator::~AudioCommentator()
{
}

void AudioCommentator::newGameWillStart()
{
    lock_guard<mutex> lock(mu_);
    texts_.push_back("さあ、ゲームの始まりです。");
}

void AudioCommentator::onUpdate(const GameState&)
{
}

void AudioCommentator::gameHasDone()
{
}

SpeakRequest AudioCommentator::requestSpeak()
{
    {
        lock_guard<mutex> lock(mu_);
        if (!texts_.empty()) {
            string textToSpeak = texts_.back();
            texts_.clear();
            return SpeakRequest { textToSpeak };
        }
    }

    vector<tuple<int, int, string>> candidates;

    string playerNames[] = {"ワンピー", "ツーピー"};

    CommentatorResult result = commentator_->result();
    for (int pi = 0; pi < 2; ++pi) {
        if (result.firingChain[pi].chains() > 0) {
            if (!firing_[pi]) {
                int chain = result.firingChain[pi].chains();
                candidates.emplace_back(100, pi, playerNames[pi] + to_string(chain) + "連鎖を発火。");
            }

            firing_[pi] = true;
        } else {
            firing_[pi] = false;
        }

        if (result.fireableMainChain[pi].chains() > 0) {
            int chain = result.fireableMainChain[pi].chains();
            candidates.emplace_back(90, pi, playerNames[pi] + to_string(chain) + "連鎖を保持中。");
        }
    }

    if (candidates.empty())
        return SpeakRequest();

    const auto& st = *max_element(candidates.begin(), candidates.end());
    return SpeakRequest(get<2>(st));
}
