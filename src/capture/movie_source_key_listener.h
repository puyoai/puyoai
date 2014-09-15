#ifndef CAPTURE_MOVIE_SOURCE_KEY_LISTENER_H_
#define CAPTURE_MOVIE_SOURCE_KEY_LISTENER_H_

#include <SDL.h>

#include "gui/main_window.h"

class MovieSource;

class MovieSourceKeyListener : public MainWindow::EventListener {
public:
    // Don't take ownership of |movieSource|.
    explicit MovieSourceKeyListener(MovieSource* movieSource) : movieSource_(movieSource) {}

    virtual void handleEvent(const SDL_Event& event) override;

private:
    MovieSource* movieSource_;
};

#endif
