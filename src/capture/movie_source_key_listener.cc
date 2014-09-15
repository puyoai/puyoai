#include "capture/movie_source_key_listener.h"

#include "capture/movie_source.h"

void MovieSourceKeyListener::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_KEYUP) {
        if (event.key.keysym.sym == SDLK_SPACE) {
            movieSource_->nextStep();
        }
    }
}


