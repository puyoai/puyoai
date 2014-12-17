#ifndef AUDIO_COMMENTATOR_H_
#define AUDIO_COMMENTATOR_H_

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "audio/speak_requester.h"
#include "base/noncopyable.h"
#include "core/core_field.h"
#include "core/kumipuyo.h"
#include "core/server/commentator.h"
#include "core/server/game_state_observer.h"

class Commentator;

class AudioCommentator : public GameStateObserver, public SpeakRequester, public CommentatorObserver, noncopyable {
public:
    AudioCommentator();
    virtual ~AudioCommentator();

    virtual void newGameWillStart() override;
    virtual void onUpdate(const GameState&) override;
    virtual void gameHasDone(GameResult) override;

    virtual void onCommentatorResultUpdate(const CommentatorResult&) override;

    virtual SpeakRequest requestSpeak();

private:
    std::mutex mu_;
    std::vector<std::string> texts_;

    std::default_random_engine rnd_;

    CommentatorResult result_;
    int penalty_[3] {};
    bool firing_[2] {};
    int lastMaxRensa_[2] {};
};

#endif
