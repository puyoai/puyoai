#ifndef DUEL_SDLGUI_MAIN_WINDOW_H_
#define DUEL_SDLGUI_MAIN_WINDOW_H_

#include <memory>
#include <vector>

#include <SDL.h>

struct Box;
class Drawer;
class GameState;
class Screen;

class MainWindow {
public:
    class EventListener {
    public:
        virtual ~EventListener() {}
        virtual void handleEvent(const SDL_Event&) {}
        virtual void handleAfterPollEvent() {}
    };

    MainWindow(int width, int height, const Box& mainBox);
    ~MainWindow();

    void runMainLoop();

    void addDrawer(Drawer*);
    void addEventListener(EventListener*);

private:
    Screen* screen() { return screen_.get(); }

    void draw();
    // Copy screen's surface to window.
    // The content of the screen might be random after this.
    void renderScreen();

    std::unique_ptr<SDL_Window, void (*)(SDL_Window*)> window_;
    std::unique_ptr<SDL_Renderer, void (*)(SDL_Renderer*)> renderer_;
    std::unique_ptr<SDL_Texture, void (*)(SDL_Texture*)> texture_;
    std::unique_ptr<Screen> screen_;

    std::vector<Drawer*> drawers_;
    std::vector<EventListener*> listeners_;

    int width_;
    int height_;
};

#endif
