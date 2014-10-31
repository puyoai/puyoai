#ifndef AUDIO_COMMENTATOR_H_
#define AUDIO_COMMENTATOR_H_

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "base/noncopyable.h"
#include "core/core_field.h"
#include "core/kumipuyo.h"
#include "core/server/commentator.h"
#include "core/server/game_state_observer.h"

class AudioServer;
class Commentator;

class AudioCommentator : public GameStateObserver, public CommentatorObserver, noncopyable {
public:
    // Doesn't take onwership
    explicit AudioCommentator(AudioServer* audioServer);
    virtual ~AudioCommentator();

    virtual void newGameWillStart() override;
    virtual void onUpdate(const GameState&) override;
    virtual void gameHasDone(GameResult) override;

    virtual void onCommentatorResultUpdate(const CommentatorResult&) override;
private:
    AudioServer* audioServer_;

    std::default_random_engine rnd_;

    std::mutex mu_;
    CommentatorResult result_;
    int penalty_[3] {};
    bool firing_[2] {};
    int lastMaxRensa_[2] {};
};

#endif
