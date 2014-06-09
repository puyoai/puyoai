#ifndef DUEL_SCREEN_H_
#define DUEL_SCREEN_H_

#include <map>
#include <string>

#include <SDL.h>
#include <SDL_ttf.h>

using namespace std;

Uint32 getpixel(SDL_Surface *surface, int x, int y);
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

class Screen {
public:
  struct Box {
    Box()
      : sx(0), sy(0), dx(0), dy(0) {}
    int size() const {
      return (dx - sx + 1) * (dy - sy + 1);
    }
    int w() const {
      return dx - sx;
    }
    int h() const {
      return dy - sy;
    }
    int sx, sy, dx, dy;
  };

  enum Shape {
    SHAPE_NONE, SHAPE_DIAMOND, SHAPE_TRIANGLE,
  };

  explicit Screen(SDL_Surface* scr);
  explicit Screen(SDL_Surface* scr, Box* bb);

  ~Screen();

  void setBoundingBox(Box* bb);
  void setMainBox(int w, int h, Box main);

  SDL_Surface* scr() { return scr_; }
  SDL_Surface* off() { return off_ ? off_ : scr_; }

  static SDL_Color bg_color() { return bg_color_; }
  static TTF_Font* font() { return font_; }

  void clear();
  void syncOffscreen();
  void flip();

  Uint32 getPixel(int x, int y);
  void putPixel(int x, int y, Uint32 pixel);

  Uint32 getColor(char c) const;

  void drawShape(Shape s, int pi, int x, int y, char c);
  void drawNumber(int pi, int x, int y, int n, char c);

  void lock();
  void unlock();

private:
  void init();

  SDL_Surface* scr_;
  SDL_Surface* off_;
  Box bb_[2][6][14];
  SDL_Color sdl_colors_[10];
  Uint32 colors_[10];
  Box main_;
  int width_, height_;
  map<pair<string, Uint32>, SDL_Surface*> font_cache_;
  bool is_synced_;
  size_t frames_;
  Uint32 ticks_[30];

  static TTF_Font* font_;
  static SDL_Color bg_color_;
};

#endif  // DUEL_SCREEN_H_
