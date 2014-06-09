#include "gui/sdl_user_input.h"

SDLUserInput::SDLUserInput()
{
#if 0
    for (int i = 0; i < SDLK_LAST; i++) {
        key_state_[i] = 0;
    }
#endif
}

Key SDLUserInput::getKey()
{
#if 0
    SDL_PumpEvents();
    Uint8* k = SDL_GetKeyState(NULL);
    for (int i = 0; i < SDLK_LAST; i++) {
      if (k[i]) {
        if (key_state_[i] >= 1)
          key_state_[i]++;
        else
          key_state_[i] = 1;
      } else {
        key_state_[i] = 0;
      }
    }

    struct {
      SDLKey sym;
      Key key;
    } KEYS[] = {
      { SDLK_x, KEY_RIGHT_TURN },
      { SDLK_z, KEY_LEFT_TURN },
      { SDLK_PERIOD, KEY_RIGHT_TURN },
      { SDLK_COMMA, KEY_LEFT_TURN },
      { SDLK_UP, KEY_UP },
      { SDLK_RIGHT, KEY_RIGHT },
      { SDLK_LEFT, KEY_LEFT },
      { SDLK_DOWN, KEY_DOWN },
      { SDLK_w, KEY_UP },
      { SDLK_d, KEY_RIGHT },
      { SDLK_a, KEY_LEFT },
      { SDLK_s, KEY_DOWN },
      { SDLK_k, KEY_UP },
      { SDLK_l, KEY_RIGHT },
      { SDLK_h, KEY_LEFT },
      { SDLK_j, KEY_DOWN },
      { SDLK_UNKNOWN, KEY_NONE }
    };

    for (size_t i = 0; KEYS[i].key != KEY_NONE; i++) {
      int ks = key_state_[KEYS[i].sym];
      Key key = KEYS[i].key;
      if (key == KEY_RIGHT_TURN || key == KEY_LEFT_TURN) {
        if (ks == 1)
          return key;
      } else if (key == KEY_DOWN) {
        if (ks > 0)
          return key;
      } else {
        if (ks == 1 || ks > 10)
          return key;
      }
    }
    return KEY_NONE;
#endif

    return KEY_NONE;
}
