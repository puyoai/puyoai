#include "capture/movie_source_key_listener.h"

#include "capture/movie_source.h"

void MovieSourceKeyListener::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_QUIT) {
        movieSource_->nextStep();
        return;
    }

    if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_SPACE) {
            movieSource_->nextStep();
            return;
        }
    }
}


