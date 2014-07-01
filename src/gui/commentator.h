#ifndef GUI_COMMENTATOR_H_
#define GUI_COMMENTATOR_H_

#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include <SDL.h>
#include <gtest/gtest.h>

#include "base/base.h"
#include "core/algorithm/rensa_info.h"
#include "core/field/core_field.h"
#include "core/kumipuyo.h"
#include "duel/game_state_observer.h"
#include "gui/drawer.h"

class Screen;

class Commentator : public GameStateObserver, public Drawer {
public:
    Commentator();
    virtual ~Commentator();

    // For GameStateObserver
    virtual void newGameWillStart() OVERRIDE;
    virtual void onUpdate(const GameState&) OVERRIDE;

    // For Drawer
    virtual void draw(Screen*) OVERRIDE;

    bool start();
    void stop();
    void runLoop();

private:
    // reset() should be called when a new game has started.
    void reset();

    void update(int pi, const CoreField&, const KumipuyoSeq&);

    void drawMainChain(Screen*) const;
    void drawCommentSurface(Screen*, int playerId) const;

    void addEventMessage(int pi, const std::string&);

    std::thread th_;
    volatile bool shouldStop_;
    volatile bool needsUpdate_[2];

    mutable std::mutex mu_;
    CoreField field_[2];
    KumipuyoSeq kumipuyoSeq_[2];
    std::string message_[2];

    std::unique_ptr<TrackedPossibleRensaInfo> fireableMainChain_[2];
    std::unique_ptr<FeasibleRensaInfo> fireableTsubushiChain_[2];
    std::unique_ptr<TrackedPossibleRensaInfo> firingChain_[2];

    std::deque<std::string> events_[2];

    FRIEND_TEST(CommentatorTest, getPotentialMaxChain);
};

#endif  // CAPTURE_COMMENTATOR_H_
