#include "screen.h"

#include <assert.h>
#include <libgen.h>
#include <limits.h>
#include <math.h>
#include <unistd.h>

#include <sstream>
#include <string>

#include <glog/logging.h>

#include "core/field.h"

using namespace std;

Uint32 getpixel(SDL_Surface *surface, int x, int y) {
  int bpp = surface->format->BytesPerPixel;
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  switch(bpp) {
    case 1:
      return *p;

    case 2:
      return *(Uint16 *)p;

    case 3:
      if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
        return p[0] << 16 | p[1] << 8 | p[2];
      else
        return p[0] | p[1] << 8 | p[2] << 16;
    case 4:
      return *(Uint32 *)p;

    default:
      return 0;
  }
}

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel) {
  int bpp = surface->format->BytesPerPixel;
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

  switch(bpp) {
    case 1:
      *p = pixel;
      break;

    case 2:
      *(Uint16 *)p = pixel;
      break;

    case 3:
      if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
        p[0] = (pixel >> 16) & 0xff;
        p[1] = (pixel >> 8) & 0xff;
        p[2] = pixel & 0xff;
      } else {
        p[0] = pixel & 0xff;
        p[1] = (pixel >> 8) & 0xff;
        p[2] = (pixel >> 16) & 0xff;
      }
      break;

    case 4:
      *(Uint32 *)p = pixel;
      break;
  }
}

TTF_Font* Screen::font_;
SDL_Color Screen::bg_color_;

Screen::Screen(SDL_Surface* scr)
  : scr_(scr),
    off_(NULL),
    is_synced_(false) {
  init();
}

Screen::Screen(SDL_Surface* scr, Box* bb)
  : scr_(scr),
    off_(NULL),
    is_synced_(false) {
  init();
  setBoundingBox(bb);
}

void Screen::init() {
  bg_color_.r = 198;
  bg_color_.g = 196;
  bg_color_.b = 156;
  //bg_color_.r = 0;
  //bg_color_.g = 59;
  //bg_color_.b = 117;

  memset(sdl_colors_, 0, sizeof(sdl_colors_));
  memset(colors_, 0, sizeof(colors_));
  if (scr_) {
    static int COLORS[][3] = {
      { 0, 0, 0 },
      { 0, 255, 255 },
      { 0, 0, 0 },
      { 255, 255, 255 },
      { 255, 0, 0 },
      { 0, 0, 255 },
      { 255, 255, 0 },
      { 0, 255, 255 },
      { 255, 0, 255 },
    };
    for (int i = 0; i < 10; i++) {
      int* c = COLORS[i];
      sdl_colors_[i].r = c[0];
      sdl_colors_[i].g = c[1];
      sdl_colors_[i].b = c[2];
      colors_[i] = SDL_MapRGB(scr_->format, c[0], c[1], c[2]);
    }
  }

  if (!TTF_WasInit()) {
    CHECK_EQ(TTF_Init(), 0) << TTF_GetError();
  }

  if (font_)
    return;

  char buf[PATH_MAX+1];
  getcwd(buf, PATH_MAX);
  char* p = buf;
  while (true) {
    string font_filename = string(p) + "/data/mikachan-p.ttf";
    if (access(font_filename.c_str(), R_OK) == 0) {
      font_ = TTF_OpenFont(font_filename.c_str(), 16);
      if (!font_) {
        LOG(FATAL) << TTF_GetError();
      }
      break;
    }

    p = dirname(p);
    if (!p[1]) {
      LOG(FATAL) << "Font not found";
    }
  }
}

Screen::~Screen() {
  //TTF_CloseFont(font_);
  for (map<pair<string, Uint32>, SDL_Surface*>::iterator iter =
         font_cache_.begin();
       iter != font_cache_.end();
       ++iter) {
    SDL_FreeSurface(iter->second);
  }
  font_cache_.clear();
  if (off_) {
    SDL_FreeSurface(off_);
  }
}

void Screen::setBoundingBox(Box* bb) {
  memcpy(bb_, bb, sizeof(bb_));
}

void Screen::setMainBox(int w, int h, Box main) {
  width_ = w;
  height_ = h;
  main_ = main;
  off_ = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h,
                              scr_->format->BitsPerPixel,
                              scr_->format->Rmask,
                              scr_->format->Gmask,
                              scr_->format->Bmask,
                              scr_->format->Amask);
}

Uint32 Screen::getPixel(int x, int y) {
  return getpixel(off(), x, y);
}

void Screen::putPixel(int x, int y, Uint32 pixel) {
  putpixel(off(), x, y, pixel);
}

Uint32 Screen::getColor(char c) const {
  assert(c >= 0);
  assert(c <= GREEN+1);
  return colors_[(int)c];
}

void Screen::drawShape(Shape s, int pi, int x, int y, char c) {
  // TODO(hamaji): Do something.
  if (y >= 12)
    return;
  Uint32 col = getColor(c);
  const Box& b = bb_[pi][x][11-y];
  switch (s) {
  case SHAPE_DIAMOND: {
    int w = (b.dx - b.sx - 2) / 2;
    int cx = (b.sx + b.dx) / 2;
    int cy = (b.sy + b.dy) / 2;
    for (int i = 0; i < w; i++) {
      putPixel(cx + i, cy - w + i, col);
      putPixel(cx + w - i, cy + i, col);
      putPixel(cx - i, cy + w - i, col);
      putPixel(cx - w + i, cy - i, col);
    }
    break;
  }

  case SHAPE_TRIANGLE: {
    int w = (b.dx - b.sx - 2);
    int cx = (b.sx + b.dx) / 2;
    int cy = (b.sy + b.dy) / 2;
    for (int i = 0; i < w; i++) {
      putPixel(cx - w / 2 + i, cy + w / sqrt(3) / 2, col);
      putPixel(cx + i / 2, cy - w / sqrt(3) + i * sqrt(3) / 2, col);
      putPixel(cx - i / 2, cy - w / sqrt(3) + i * sqrt(3) / 2, col);
    }
    break;
  }

  case SHAPE_NONE:
    break;

  default:
    assert(0);
  }
}

void Screen::drawNumber(int pi, int x, int y, int n, char /*c*/) {
  Uint32 col = getColor(3);
  SDL_Color scol = sdl_colors_[3];
  ostringstream oss;
  oss << n;
  pair<string, Uint32> key = make_pair(oss.str(), col);
  pair<map<pair<string, Uint32>, SDL_Surface*>::iterator, bool> p =
    font_cache_.insert(make_pair(key, (SDL_Surface*)0));
  if (p.second) {
    SDL_Surface* t = TTF_RenderUTF8_Blended(font_, oss.str().c_str(), scol);
    SDL_Surface* s = SDL_CreateRGBSurface(SDL_SWSURFACE, 20, 20, 32,
                                          0, 0, 0, 0);
    SDL_SetColorKey(s, SDL_SRCCOLORKEY, SDL_MapRGB(s->format, 0, 0, 0));

    SDL_Rect dr;
    dr.x = 12;
    dr.y = 10;
    dr.w = 8;
    dr.h = 8;
    SDL_FillRect(s, &dr, SDL_MapRGB(s->format, 1, 1, 1));

    dr.x = s->w - t->w;
    dr.y = s->h - t->h;
    SDL_BlitSurface(t, NULL, s, &dr);
    //p.first->second = s;
    p.first->second = SDL_DisplayFormat(s);
    SDL_FreeSurface(s);
    SDL_FreeSurface(t);
  }

  SDL_Surface* s = p.first->second;
  SDL_Surface* dst;
  SDL_Rect dr;
  const Box& b = bb_[pi][x][11-y];
  if (is_synced_) {
    dr.x = main_.sx + b.dx * main_.w() / width_ - s->w;
    dr.y = main_.sy + b.dy * main_.h() / height_ - s->h;
    dst = scr_;
  } else {
    dr.x = b.dx - s->w;
    dr.y = b.dy - s->h;
    dst = off();
  }
  dr.x += 5;
  dr.y -= 4;
  SDL_BlitSurface(s, NULL, dst, &dr);
}

void Screen::clear() {
  if (off_) {
    SDL_FillRect(scr_, NULL, SDL_MapRGB(
                   scr_->format, bg_color_.r, bg_color_.g, bg_color_.b));
  }
}

void Screen::syncOffscreen() {
  if (off_) {
    // Maybe better to use libswscale.
    SDL_Rect sr = { 0, 0, width_, height_ };
    SDL_Rect dr = { main_.sx, main_.sy, main_.w(), main_.h() };
    SDL_SoftStretch(off_, &sr, scr_, &dr);
  }
  is_synced_ = true;
}

void Screen::flip() {
  is_synced_ = false;
  //Uint32 tck = SDL_GetTicks();
  SDL_Flip(scr_);
  //fprintf(stderr, "%u\n", SDL_GetTicks() - tck);
}

void Screen::lock() {
  SDL_LockSurface(off());
}

void Screen::unlock() {
  SDL_UnlockSurface(off());
}
