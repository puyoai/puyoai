#ifndef GUI_USER_EVENT_DRAWER_H_
#define GUI_USER_EVENT_DRAWER_H_

#include <mutex>

#include "core/server/game_state_observer.h"
#include "core/user_event.h"
#include "gui/drawer.h"

// UserEventDrawer draws the user event set.
class UserEventDrawer : public Drawer, public GameStateObserver {
public:
    // Don't take ownership
    UserEventDrawer() {}
    ~UserEventDrawer() override {}

    virtual void onUpdate(const GameState&) override;
    virtual void draw(Screen*) override;

private:
    mutable std::mutex mu_;
    UserEvent userEvents_[2];
};

#endif // GUI_USER_EVENT_DRAWER_H_
