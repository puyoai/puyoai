#ifndef AUDIO_COMMENTATOR_H_
#define AUDIO_COMMENTATOR_H_

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "base/base.h"
#include "duel/game_state_observer.h"

class Speaker;

class AudioCommentator : public GameStateObserver, noncopyable {
public:
    // Does not take the ownership. |speaker| should be alive
    // AudioCommentator is alive.
    explicit AudioCommentator(Speaker* speaker);
    virtual ~AudioCommentator();

    virtual void newGameWillStart() override;
    virtual void onUpdate(const GameState&) override;
    virtual void gameHasDone() override;

    void start();
    void stop();

    void addText(const std::string&);

private:
    void runLoop();

    std::atomic<bool> shouldStop_;
    std::thread th_;
    std::mutex mu_;
    std::condition_variable cond_;
    std::vector<std::string> texts_;

    Speaker* speaker_ = nullptr;
};

#endif
