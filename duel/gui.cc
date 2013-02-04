#include "gui.h"

#ifdef USE_SDL

#include <stdio.h>
#include <stdlib.h>

#include <iomanip>
#include <sstream>

#include <SDL.h>

#include <glog/logging.h>

#include "SDL_kanji.h"
#include "field_realtime.h"
#include "screen.h"

using namespace std;

const char* kJapaneseBdfName = "data/jiskan24.bdf";
const char* kEnglishBdfName = "data/12x24.bdf";
const int kBdfSize = 24;

class GuiSdl : public Gui {
public:
  GuiSdl() {
    SDL_Init(SDL_INIT_VIDEO);
    bool use_commentator =
      !getenv("PUYO_COMMENTATOR") || strcmp(getenv("PUYO_COMMENTATOR"), "0");
    if (use_commentator) {
      SDL_Surface* scr = SDL_SetVideoMode(800, 600, 32,
                                          SDL_SWSURFACE | SDL_DOUBLEBUF);
      Screen::Box b;
      b.sx = 144;
      b.dx = 656;
      b.sy = 40;
      b.dy = 424;
      scr_ = new Screen(scr);
      scr_->setMainBox(512, 384, b);
    } else {
      SDL_Surface* scr = SDL_SetVideoMode(512, 384, 32,
                                          SDL_SWSURFACE | SDL_DOUBLEBUF);
      scr_ = new Screen(scr);
    }

    font_ = Kanji_OpenFont(kJapaneseBdfName, kBdfSize);
    Kanji_AddFont(font_, kEnglishBdfName);
    CHECK(font_ != NULL) << "Failed to load fonts";

    for (int i = 0; i < SDLK_LAST; i++) {
      key_state_[i] = 0;
    }
  }

  virtual ~GuiSdl() {
    SDL_Quit();
  }

  virtual void Draw(const FieldRealtime& field,
                    const string& debug_message) {
#ifdef USE_SDL
    int x1, y1, x2, y2, r;
    char c1, c2;
    field.GetCurrentPuyo(&x1, &y1, &c1, &x2, &y2, &c2, &r);

    int pos_x = 1 + 13 * field.player_id();
    int pos_y = 1;
    for (int y = 0; y < Field::MAP_HEIGHT; y++) {
      for (int x = 0; x < Field::MAP_WIDTH; x++) {
        SDL_Rect r;
        r.x = (pos_x + x) * 20;
        r.y = (pos_y + Field::MAP_HEIGHT - y) * 20;
        r.w = 20;
        r.h = 20;

        char color = field.Get(x, y);
        if (field.IsInUserState()) {
          if (x == x1 && y == y1) {
            color = c1;
          }
          if (x == x2 && y == y2) {
            color = c2;
          }
        }

        Uint32 c = GetPuyoColor(color);
        SDL_FillRect(scr_->off(), &r, c);
      }
    }

    // Death line
    {
      SDL_Rect r;
      r.x = pos_x * 20;
      r.y = (pos_y + 4) * 20;
      r.w = 20 * 8;
      r.h = 1;
      SDL_FillRect(scr_->off(), &r,
                   SDL_MapRGB(scr_->off()->format, 255, 255, 255));
    }

    // Next puyo info
    for (int i = 2; i < 6; i++) {
      SDL_Rect r;
      r.x = (pos_x + 9) * 20;
      r.y = (pos_y + + 3 + (i - 2) + ((i - 2) / 2)) * 20;
      r.w = 20;
      r.h = 20;
      Uint32 c = GetPuyoColor(field.GetNextPuyo(i));
      SDL_FillRect(scr_->off(), &r, c);
    }


    SDL_Color white;
    white.r = white.g = white.b = 255;

    // Score
    {
      ostringstream ss;
      ss << setw(10) << field.GetScore();
      Kanji_PutText(font_, pos_x * 20, (pos_y + Field::MAP_HEIGHT + 1) * 20,
                    scr_->off(), ss.str().c_str(), white);
    }

#if 0
    // Debug message
    {
      ostringstream ss;
      ss << setw(10) << field.GetScore();
      Kanji_PutText(font_, pos_x * 20, (pos_y + Field::MAP_HEIGHT + 1) * 20,
                    scr_->off(), ss.str().c_str(), white);
    }
#endif

    // Ojama
    {
      ostringstream ss;
      ss << field.GetFixedOjama() << '(' << field.GetPendingOjama() << ')';
      Kanji_PutText(font_, pos_x * 20, pos_y * 20,
                    scr_->off(), ss.str().c_str(), white);
    }

    {
      int y = pos_y + Field::MAP_HEIGHT + 3 + field.player_id();
      Kanji_PutText(font_, 20, y * 20, scr_->off(), debug_message.c_str(), white);
    }

#endif
  }

  virtual void Flip() {
    scr_->syncOffscreen();
    scr_->flip();
    SDL_FillRect(scr_->off(), NULL, SDL_MapRGB(scr_->off()->format, 0, 0, 0));
    scr_->clear();
  }

  virtual Key GetKey() {
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
  }

private:
  Uint32 GetPuyoColor(char color) {
    Uint32 c = 0;
    switch (color) {
    case EMPTY:
      c = SDL_MapRGB(scr_->off()->format, 0, 0, 0);
      break;
    case OJAMA:
      c = SDL_MapRGB(scr_->off()->format, 127, 127, 127);
      break;
    case WALL:
      c = SDL_MapRGB(scr_->off()->format, 255, 255, 255);
      break;
    case RED:
      c = SDL_MapRGB(scr_->off()->format, 255, 0, 0);
      break;
    case BLUE:
      c = SDL_MapRGB(scr_->off()->format, 0, 0, 255);
      break;
    case YELLOW:
      c = SDL_MapRGB(scr_->off()->format, 255, 255, 0);
      break;
    case GREEN:
      c = SDL_MapRGB(scr_->off()->format, 0, 255, 0);
      break;
    default:
      break;
    }
    return c;
  }

  Screen* scr_;
  Kanji_Font* font_;
  int key_state_[SDLK_LAST];
};

#endif

Gui* Gui::Create() {
#ifdef USE_SDL
  return new GuiSdl;
#else
  return new Gui;
#endif
}
