#ifndef AUDIO_COMMENTATOR_H_
#define AUDIO_COMMENTATOR_H_

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "audio/speak_requester.h"
#include "base/base.h"
#include "core/field/core_field.h"
#include "core/kumipuyo.h"
#include "duel/game_state_observer.h"

class Commentator;

class AudioCommentator : public GameStateObserver, public SpeakRequester, noncopyable {
public:
    // Does not take the ownership. |speaker| should be alive
    // AudioCommentator is alive.
    AudioCommentator(const Commentator* commentator);
    virtual ~AudioCommentator();

    virtual void newGameWillStart() override;
    virtual void onUpdate(const GameState&) override;
    virtual void gameHasDone() override;

    virtual SpeakRequest requestSpeak();

private:
    std::mutex mu_;

    const Commentator* commentator_ = nullptr;
    std::vector<std::string> texts_;
};

#endif
