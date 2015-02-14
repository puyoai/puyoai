#ifndef GUI_FIELD_DRAWER_H_
#define GUI_FIELD_DRAWER_H_

#include <memory>
#include <mutex>

#include "base/base.h"
#include "core/server/game_state_observer.h"
#include "gui/box.h"
#include "gui/drawer.h"
#include "gui/screen.h"
#include "gui/unique_sdl_surface.h"
#include "gui/SDL_kanji.h"

class GameState;
class MainWindow;
struct PlayerGameState;
class SDLCommentator;

// FieldDrawer draws the current puyo field etc.
class FieldDrawer : public Drawer, public GameStateObserver {
public:
    // Don't take ownership
    FieldDrawer();
    virtual ~FieldDrawer() override;

    virtual void onInit() override;
    virtual void onUpdate(const GameState&) override;
    virtual void draw(Screen*) override;

private:
    void drawField(Screen*, int playerId, const PlayerGameState&);
    SDL_Rect toRect(PuyoColor);

    mutable std::mutex mu_;
    std::unique_ptr<GameState> gameState_;

    UniqueSDLSurface backgroundSurface_;
    UniqueSDLSurface puyoSurface_;
    UniqueSDLSurface ojamaSurface_;

    Kanji_Font* font_;
};

#endif
