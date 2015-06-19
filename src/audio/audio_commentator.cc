#include "audio/audio_commentator.h"

#include <algorithm>
#include <iostream>
#include "glog/logging.h"

#include "audio/audio_server.h"
#include "base/strings.h"
#include "core/server/commentator.h"
#include "core/server/game_state.h"

using namespace std;

AudioCommentator::AudioCommentator(AudioServer* audioServer) :
    audioServer_(audioServer),
    rnd_(random_device()())
{
}

AudioCommentator::~AudioCommentator()
{
}

void AudioCommentator::newGameWillStart()
{
    std::uniform_int_distribution<int> uid(0, 2);
    lock_guard<mutex> lock(mu_);
    switch (uid(rnd_)) {
    case 0:
        audioServer_->submit("さあ、ゲームの始まりです。");
        break;
    case 1:
        audioServer_->submit("始まりましたよ、おくさん。");
        break;
    case 2:
        audioServer_->submit("はじまりました。", 1, 5);
        break;
    }
}

void AudioCommentator::onUpdate(const GameState&)
{
}

void AudioCommentator::gameHasDone(GameResult gameResult)
{
    switch (gameResult) {
    case GameResult::P1_WIN:
        audioServer_->submit("ワンピーの勝ち");
        break;
    case GameResult::P2_WIN:
        audioServer_->submit("ツーピーの勝ち");
        break;
    default:
        break;
    }
}

void AudioCommentator::onCommentatorResultUpdate(const CommentatorResult& result)
{
    vector<tuple<int, int, string>> candidates;

    string playerNames[] = {"ワンピー", "ツーピー"};

    for (int pi = 0; pi < 2; ++pi) {
        if (result.firingChain[pi].chains() > 0) {
            if (!firing_[pi]) {
                int chain = result.firingChain[pi].chains();
                int score = result.firingChain[pi].score();
                if (chain == 1) {
                    if (score >= 70 * 10) {
                        candidates.emplace_back(score - penalty_[pi], pi, playerNames[pi] + "、イバラを発火");
                    }
                    if (score <= 100) {
                        candidates.emplace_back(3 - penalty_[pi], pi, playerNames[pi] + "、単発で整地");
                    }
                } else {
                    string s = playerNames[pi] + to_string(chain) + "連鎖を発火。";
                    if (score / 70 >= 6)
                        s += "オジャマプヨ " + to_string(score / 70) + "個が発生";
                    if (firing_[1 - pi]) {
                        int opponentScore = result.firingChain[1 - pi].score();
                        if (score > opponentScore + 70 * 30) {
                            s += "十分な大きさ。";
                        } else if (score > opponentScore - 70 * 12) {
                            s += "なんとか対応。";
                        } else {
                            s += "これでは小さい。";
                        }
                    }
                    candidates.emplace_back(score, pi, s);
                }
            }

            firing_[pi] = true;
        } else {
            firing_[pi] = false;
        }

        if (result.fireableMainChain[pi].chains() > 0) {
            int chain = result.fireableMainChain[pi].chains();
            int priority = chain - penalty_[pi];
            if (lastMaxRensa_[pi] < chain) {
                candidates.emplace_back(priority, pi, playerNames[pi] + to_string(chain) + "連鎖を構築。");
                lastMaxRensa_[pi] = chain;
            }
        }

        lastMaxRensa_[pi] = result.fireableMainChain[pi].chains();
    }

    if (!firing_[0] && !firing_[1]) {
        int score0 = result.fireableMainChain[0].score();
        int score1 = result.fireableMainChain[1].score();
        if (score0 > score1 - 30 * 70 && score0 > score1 * 2) {
            candidates.emplace_back(12 - penalty_[2], 2,
                                    playerNames[0] + "のほうが" + playerNames[1] + "をリード。");
        }

        if (score1 > score0 - 30 * 70 && score1 > score0 * 2) {
            candidates.emplace_back(12 - penalty_[2], 2,
                                    playerNames[1] + "のほうが" + playerNames[0] + "をリード。");
        }

    }

    if (candidates.empty())
        return;

    const auto& st = *max_element(candidates.begin(), candidates.end());
    switch (get<1>(st)) {
    case 0:
        penalty_[1] = penalty_[2] = 0;
        penalty_[0]--;
        break;
    case 1:
        penalty_[0] = penalty_[2] = 0;
        penalty_[1]--;
        break;
    case 2:
        penalty_[0] = penalty_[1] = 0;
        penalty_[2] = 12;
        break;
    }

    audioServer_->submit(get<2>(st));
}
