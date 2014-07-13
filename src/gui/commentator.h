#ifndef GUI_COMMENTATOR_H_
#define GUI_COMMENTATOR_H_

#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include <SDL.h>
#include <gtest/gtest.h>

#include "core/algorithm/rensa_info.h"
#include "core/field/core_field.h"
#include "core/kumipuyo.h"
#include "duel/game_state_observer.h"
#include "gui/drawer.h"

class Screen;

struct CommentatorResult {
    int frameId[2];
    TrackedPossibleRensaInfo fireableMainChain[2];
    FeasibleRensaInfo fireableTsubushiChain[2];
    TrackedPossibleRensaInfo firingChain[2];
    std::string message[2];

    std::deque<std::string> events[2];
};

class Commentator : public GameStateObserver {
public:
    Commentator();
    virtual ~Commentator();

    // For GameStateObserver
    virtual void newGameWillStart() override;
    virtual void onUpdate(const GameState&) override;

    CommentatorResult result() const;

    bool start();
    void stop();
    void runLoop();

private:
    // reset() should be called when a new game has started.
    void reset();

    void update(int pi, const CoreField&, const KumipuyoSeq&);

    void addEventMessage(int pi, const std::string&);

    std::thread th_;
    volatile bool shouldStop_;
    volatile bool needsUpdate_[2];

    mutable std::mutex mu_;
    CoreField field_[2];
    KumipuyoSeq kumipuyoSeq_[2];
    std::string message_[2];

    int frameId_[2];
    std::unique_ptr<TrackedPossibleRensaInfo> fireableMainChain_[2];
    std::unique_ptr<FeasibleRensaInfo> fireableTsubushiChain_[2];
    std::unique_ptr<TrackedPossibleRensaInfo> firingChain_[2];

    std::deque<std::string> events_[2];

    FRIEND_TEST(CommentatorTest, getPotentialMaxChain);
};

class CommentatorDrawer : public Drawer {
public:
    explicit CommentatorDrawer(const Commentator*);
    virtual ~CommentatorDrawer();

    virtual void draw(Screen*) override;

private:
    void drawMainChain(Screen*, const CommentatorResult&) const;
    void drawCommentSurface(Screen*, const CommentatorResult&, int playerId) const;

    const Commentator* commentator_;
};

#endif  // CAPTURE_COMMENTATOR_H_
