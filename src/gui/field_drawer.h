#ifndef DUEL_FIELD_DRAWER_H_
#define DUEL_FIELD_DRAWER_H_

#include <memory>

#include "base/base.h"
#include "base/lock.h"
#include "duel/game_state_observer.h"
#include "gui/box.h"
#include "gui/drawer.h"
#include "gui/screen.h"
#include "gui/SDL_kanji.h"

class FieldRealtime;
class GameState;
class MainWindow;
class SDLCommentator;

// FieldDrawer draws the current puyo field etc.
class FieldDrawer : public GameStateObserver, public Drawer {
public:
    // Don't take ownership
    FieldDrawer();
    virtual ~FieldDrawer();

    virtual void onInit(Screen*) OVERRIDE;
    virtual void onUpdate(const GameState&) OVERRIDE;
    virtual void draw(Screen*) OVERRIDE;

private:
    void drawField(Screen*, const FieldRealtime&);
    // TODO(mayah): Why char? Why not PuyoColor?
    Uint32 GetPuyoColor(SDL_Surface*, char color) const;

    mutable Mutex mu_;
    std::unique_ptr<GameState> gameState_;

    Kanji_Font* font_;
};

#endif
