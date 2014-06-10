#include "gui/main_window.h"

#include "duel/game_state.h"
#include "gui/drawer.h"
#include "gui/screen.h"

MainWindow::MainWindow(int width, int height, const Box& mainBox) :
    window_(nullptr, SDL_DestroyWindow),
    renderer_(nullptr, SDL_DestroyRenderer),
    texture_(nullptr, SDL_DestroyTexture),
    width_(width),
    height_(height)
{
    screen_.reset(new Screen(width_, height_, mainBox));
}

MainWindow::~MainWindow()
{
}

void MainWindow::runMainLoop()
{
    window_.reset(SDL_CreateWindow("puyo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width_, height_, SDL_WINDOW_SHOWN));
    renderer_.reset(SDL_CreateRenderer(window_.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
    texture_.reset(SDL_CreateTexture(renderer_.get(), SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width_, height_));

    bool shouldStop = false;
    while (!shouldStop) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                shouldStop = true;
                break;
            case SDL_KEYDOWN: {
                SDL_Keycode key = ev.key.keysym.sym;
                if (key == SDLK_ESCAPE)
                    shouldStop = true;
                break;
            }
            default:
                break;
            }

            for (auto listener : listeners_)
                listener->handleEvent(ev);
        }

        draw();
    }
}

void MainWindow::addDrawer(Drawer* drawer)
{
    drawers_.push_back(drawer);
    drawer->onInit(screen());
}

void MainWindow::addEventListener(EventListener* listener)
{
    listeners_.push_back(listener);
}

void MainWindow::draw()
{
    screen()->clear();

    for (Drawer* drawer : drawers_)
        drawer->draw(screen());

    renderScreen();
}

void MainWindow::renderScreen()
{
    SDL_Surface* surface = screen()->surface();

    SDL_UpdateTexture(texture_.get(), nullptr, surface->pixels, surface->pitch);
    SDL_RenderClear(renderer_.get());
    SDL_RenderCopy(renderer_.get(), texture_.get(), nullptr, nullptr);
    SDL_RenderPresent(renderer_.get());
}
