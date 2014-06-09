#ifndef DUEL_SDL_USER_INPUT_H_
#define DUEL_SDL_USER_INPUT_H_

#include <SDL.h>

#include "base/base.h"
#include "duel/user_input.h"

class SDLUserInput : public UserInput {
public:
    SDLUserInput();
    virtual ~SDLUserInput() {}

    virtual Key getKey() OVERRIDE;

private:
    // int key_state_[SDLK_LAST];
};

#endif
