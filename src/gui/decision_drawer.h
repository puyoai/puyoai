#ifndef GUI_DECISION_DRAWER_H_
#define GUI_DECISION_DRAWER_H_

#include <mutex>

#include "core/decision.h"
#include "core/kumipuyo.h"
#include "core/kumipuyo_pos.h"
#include "core/server/game_state_observer.h"
#include "gui/drawer.h"
#include "gui/screen.h"

class DecisionDrawer : public Drawer, public GameStateObserver {
public:
    DecisionDrawer() {}
    virtual ~DecisionDrawer() override {}

    virtual void onUpdate(const GameState&) override;
    virtual void draw(Screen*) override;

private:
    mutable std::mutex mu_;
    KumipuyoPos pos_[2];
    Kumipuyo kumipuyo_[2];
};

#endif
